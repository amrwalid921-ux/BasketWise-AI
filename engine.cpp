// BasketWise AI engine: Merge Sort + Binary Search + A* + Knapsack(DP) vs Greedy + NN/2-opt
// Build: g++ -O2 -std=c++17 engine.cpp -o engine
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <bits/stdc++.h>
using namespace std;
struct P { int id; string name, cat; int price; double rating; int x, y; double score = 0; };
vector<P> prods; vector<string> grid; int W, H;
typedef pair<int,int> pii;

string low(string s){ for(auto&c:s) c=tolower(c); return s; }

void loadData(){
  ifstream f("data/products.txt"); string l;
  while(getline(f,l)){
    if(l.empty()||l[0]=='#') continue;
    stringstream ss(l); string t[7];
    for(int i=0;i<7;i++) getline(ss,t[i],'|');
    prods.push_back({stoi(t[0]),t[1],t[2],stoi(t[3]),stod(t[4]),stoi(t[5]),stoi(t[6])});
  }
  ifstream m("data/map.txt");
  while(getline(m,l)) if(!l.empty()) grid.push_back(l);
  H=grid.size(); W=grid[0].size();
}

// ---------- 1) Sorting: Merge Sort (stable, O(n log n)) ----------
void msort(vector<P>&a,int l,int r,function<bool(const P&,const P&)> lt){
  if(r-l<2) return;
  int m=(l+r)/2; msort(a,l,m,lt); msort(a,m,r,lt);
  vector<P> t; int i=l,j=m;
  while(i<m&&j<r) t.push_back(lt(a[j],a[i])?a[j++]:a[i++]);
  while(i<m) t.push_back(a[i++]);
  while(j<r) t.push_back(a[j++]);
  copy(t.begin(),t.end(),a.begin()+l);
}

// ---------- 2) Searching: Binary Search on sorted names (prefix) ----------
vector<P> searchName(string q){
  vector<P> s=prods;
  msort(s,0,s.size(),[](const P&a,const P&b){return low(a.name)<low(b.name);});
  q=low(q); int lo=0,hi=s.size();
  while(lo<hi){ int m=(lo+hi)/2; if(low(s[m].name)<q) lo=m+1; else hi=m; }
  vector<P> r;
  for(;lo<(int)s.size()&&low(s[lo].name).compare(0,q.size(),q)==0;lo++) r.push_back(s[lo]);
  return r;
}

// ---------- 3) Shortest Path: A* with Manhattan heuristic ----------
struct Res { int d; vector<pii> path; int expanded; };
Res astar(int sx,int sy,int gx,int gy,bool useH=true){
  vector<vector<int>> g(H,vector<int>(W,1e9));
  vector<vector<pii>> par(H,vector<pii>(W,{-1,-1}));
  typedef tuple<int,int,int> T;
  priority_queue<T,vector<T>,greater<T>> pq;
  auto h=[&](int x,int y){return useH?abs(x-gx)+abs(y-gy):0;};
  g[sy][sx]=0; pq.push({h(sx,sy),sx,sy}); int ex=0;
  int dx[]={1,-1,0,0}, dy[]={0,0,1,-1};
  while(!pq.empty()){
    auto [f,x,y]=pq.top(); pq.pop();
    if(f>g[y][x]+h(x,y)) continue;
    ex++; if(x==gx&&y==gy) break;
    for(int k=0;k<4;k++){
      int nx=x+dx[k], ny=y+dy[k];
      if(nx<0||ny<0||nx>=W||ny>=H||grid[ny][nx]=='#') continue;
      if(g[y][x]+1<g[ny][nx]){ g[ny][nx]=g[y][x]+1; par[ny][nx]={x,y}; pq.push({g[ny][nx]+h(nx,ny),nx,ny}); }
    }
  }
  Res r; r.d=g[gy][gx]; r.expanded=ex;
  if(r.d<1e9){
    for(pii c={gx,gy}; c.first!=-1; c=par[c.second][c.first]) r.path.push_back(c);
    reverse(r.path.begin(),r.path.end());
  }
  return r;
}

string pj(const P&p){
  ostringstream o;
  o<<"{\"id\":"<<p.id<<",\"name\":\""<<p.name<<"\",\"cat\":\""<<p.cat<<"\",\"price\":"<<p.price
   <<",\"rating\":"<<p.rating<<",\"x\":"<<p.x<<",\"y\":"<<p.y<<",\"score\":"<<fixed<<setprecision(3)<<p.score<<"}";
  return o.str();
}
string plist(const vector<P>&v){
  string s="["; for(size_t i=0;i<v.size();i++){ if(i) s+=","; s+=pj(v[i]); } return s+"]";
}


int V(const P&p){return (int)(p.score*1000);}
double nowms(){
#ifdef _WIN32
  LARGE_INTEGER f,c; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&c);
  return c.QuadPart*1000.0/f.QuadPart;
#else
  return chrono::duration<double,milli>(chrono::steady_clock::now().time_since_epoch()).count();
#endif
}
int knapDP(const vector<P>&a,int B){vector<int> d(B+1,0);for(auto&p:a)for(int c=B;c>=p.price;c--)d[c]=max(d[c],d[c-p.price]+V(p));return d[B];}
int knapGreedy(vector<P> a,int B){msort(a,0,a.size(),[](const P&x,const P&y){return V(x)/(double)x.price>V(y)/(double)y.price;});int v=0;for(auto&p:a)if(p.price<=B){B-=p.price;v+=V(p);}return v;}
// AI: Genetic Algorithm (tournament selection, 1-point crossover, mutation, repair, elitism)
int genetic(const vector<P>&a,int B){
  int n=a.size(),PS=100,G=800; mt19937 rng(42); typedef vector<char> Ch;
  auto repair=[&](Ch&c){int w=0;for(int i=0;i<n;i++)if(c[i])w+=a[i].price;while(w>B){int i=rng()%n;if(c[i]){c[i]=0;w-=a[i].price;}}};
  auto fit=[&](const Ch&c){int v=0;for(int i=0;i<n;i++)if(c[i])v+=V(a[i]);return v;};
  vector<Ch> pop(PS,Ch(n)); for(auto&c:pop){for(int i=0;i<n;i++)c[i]=rng()%10==0;repair(c);}
  Ch best=pop[0]; int bf=0;
  for(int g=0;g<G;g++){
    vector<int> f(PS); for(int i=0;i<PS;i++){f[i]=fit(pop[i]);if(f[i]>bf){bf=f[i];best=pop[i];}}
    vector<Ch> nx={best};
    auto tour=[&](){int x=rng()%PS,y=rng()%PS;return f[x]>f[y]?x:y;};
    while((int)nx.size()<PS){Ch c=pop[tour()],d=pop[tour()];int cut=rng()%n;for(int i=cut;i<n;i++)c[i]=d[i];for(int i=0;i<n;i++)if(rng()%200<1)c[i]=!c[i];repair(c);nx.push_back(c);}
    pop=nx;
  }
  return bf;
}
void qsortP(vector<P>&a,int l,int r){while(l<r){int i=l,j=r;double pv=a[(l+r)/2].score;while(i<=j){while(a[i].score>pv)i++;while(a[j].score<pv)j--;if(i<=j)swap(a[i++],a[j--]);}if(j-l<r-i){qsortP(a,l,j);l=i;}else{qsortP(a,i,r);r=j;}}}
// Benchmarks for the report: Merge vs Quick, DP vs Greedy vs Genetic, Dijkstra vs A*
void bench(){
  mt19937 rng(7); ostringstream o; o<<"{\"sort\":[";
  int sizes[]={2000,10000,50000};
  for(int s=0;s<3;s++){int n=sizes[s];vector<P> a(n);for(auto&p:a){p.price=5+rng()%95;p.score=(rng()%100000)/100000.0;}
    auto b=a;double t=nowms();msort(b,0,n,[](const P&x,const P&y){return x.score>y.score;});double tm=nowms()-t;
    b=a;t=nowms();qsortP(b,0,n-1);double tq=nowms()-t;
    o<<(s?",":"")<<"{\"n\":"<<n<<",\"merge\":"<<tm<<",\"quick\":"<<tq<<"}";}
  vector<P> it(200);for(auto&p:it){p.price=5+rng()%95;p.score=(rng()%100000)/100000.0;}
  double t=nowms();int dv=knapDP(it,1000);double td=nowms()-t;
  t=nowms();int gv=knapGreedy(it,1000);double tg=nowms()-t;
  t=nowms();int av=genetic(it,1000);double ta=nowms()-t;
  o<<"],\"knap\":{\"dp\":["<<dv<<","<<td<<"],\"greedy\":["<<gv<<","<<tg<<"],\"ga\":["<<av<<","<<ta<<"]},";
  int ex=0,ey=0;for(int y=0;y<H;y++)for(int x=0;x<W;x++)if(grid[y][x]=='E'){ex=x;ey=y;}
  long ea=0,ed=0;for(auto&p:prods){ea+=astar(ex,ey,p.x,p.y,true).expanded;ed+=astar(ex,ey,p.x,p.y,false).expanded;}
  o<<"\"path\":{\"astar\":"<<ea<<",\"dijkstra\":"<<ed<<"}}";
  cout<<o.str()<<endl;
}

void plan(int budget,string cats){
  budget=max(0,min(budget,2000));
  set<string> pref; { stringstream ss(cats); string c; while(getline(ss,c,',')) pref.insert(c); }
  // Utility-based scoring (intelligent agent maximizing user utility)
  for(auto&p:prods) p.score=0.5*p.rating/5+0.3/(1+p.price/50.0)+0.2*pref.count(p.cat);
  auto ranked=prods;
  msort(ranked,0,ranked.size(),[](const P&a,const P&b){return a.score>b.score;});
  int n=ranked.size();
  auto val=[&](const P&p){return (int)(p.score*1000);};

  // AI choice A: 0/1 Knapsack by Dynamic Programming (optimal)
  vector<vector<int>> dp(n+1,vector<int>(budget+1,0));
  for(int i=1;i<=n;i++) for(int c=0;c<=budget;c++){
    dp[i][c]=dp[i-1][c];
    int pr=ranked[i-1].price;
    if(pr<=c) dp[i][c]=max(dp[i][c],dp[i-1][c-pr]+val(ranked[i-1]));
  }
  vector<P> basket; int total=0;
  for(int i=n,c=budget;i>0;i--) if(dp[i][c]!=dp[i-1][c]){ basket.push_back(ranked[i-1]); c-=ranked[i-1].price; total+=ranked[i-1].price; }

  // AI choice B: Greedy by value/price (fast, approximate)
  auto gr=prods;
  msort(gr,0,n,[&](const P&a,const P&b){return val(a)/(double)a.price>val(b)/(double)b.price;});
  int gLeft=budget,gVal=0,gCnt=0;
  for(auto&p:gr) if(p.price<=gLeft){ gLeft-=p.price; gVal+=val(p); gCnt++; }

  // Route: A* distance matrix -> Nearest Neighbor -> 2-opt (closed tour from/to entrance)
  int ex=0,ey=0; for(int y=0;y<H;y++) for(int x=0;x<W;x++) if(grid[y][x]=='E'){ex=x;ey=y;}
  int k=basket.size(); vector<pii> pts={{ex,ey}};
  for(auto&b:basket) pts.push_back({b.x,b.y});
  vector<vector<int>> D(k+1,vector<int>(k+1,0));
  for(int i=0;i<=k;i++) for(int j=0;j<=k;j++) if(i!=j) D[i][j]=astar(pts[i].first,pts[i].second,pts[j].first,pts[j].second).d;
  vector<int> t={0}; vector<bool> used(k+1,false); used[0]=true;
  for(int s=0;s<k;s++){ int cur=t.back(),best=-1; for(int j=1;j<=k;j++) if(!used[j]&&(best<0||D[cur][j]<D[cur][best])) best=j; used[best]=true; t.push_back(best); }
  auto len=[&](const vector<int>&v){int s=0; for(size_t i=0;i<v.size();i++) s+=D[v[i]][v[(i+1)%v.size()]]; return s;};
  int nnLen=len(t); bool imp=true;
  while(imp){ imp=false;
    for(int i=1;i<=k;i++) for(int j=i+1;j<=k;j++){ auto u=t; reverse(u.begin()+i,u.begin()+j+1); if(len(u)<len(t)){t=u;imp=true;} } }
  vector<pii> path; int expanded=0;
  for(size_t i=0;i<t.size();i++){
    auto r=astar(pts[t[i]].first,pts[t[i]].second,pts[t[(i+1)%t.size()]].first,pts[t[(i+1)%t.size()]].second);
    expanded+=r.expanded;
    for(size_t j=(path.empty()?0:1);j<r.path.size();j++) path.push_back(r.path[j]);
  }
  ostringstream o;
  o<<"{\"grid\":["; for(int i=0;i<H;i++) o<<(i?",":"")<<"\""<<grid[i]<<"\"";
  o<<"],\"ranked\":"<<plist(ranked)<<",\"basket\":"<<plist(basket)<<",\"total\":"<<total
   <<",\"dpValue\":"<<dp[n][budget]<<",\"greedyValue\":"<<gVal<<",\"greedyCount\":"<<gCnt<<",\"gaValue\":"<<genetic(ranked,budget)
   <<",\"nnSteps\":"<<nnLen<<",\"steps\":"<<len(t)<<",\"expanded\":"<<expanded<<",\"order\":[";
  for(int i=1;i<(int)t.size();i++) o<<(i>1?",":"")<<basket[t[i]-1].id;
  o<<"],\"path\":["; for(size_t i=0;i<path.size();i++) o<<(i?",":"")<<"["<<path[i].first<<","<<path[i].second<<"]";
  o<<"]}";
  cout<<o.str()<<endl;
}

int main(int argc,char**argv){
  loadData();
  string cmd=argc>1?argv[1]:"";
  if(cmd=="search") cout<<plist(searchName(argc>2?argv[2]:""))<<endl;
  else if(cmd=="plan") plan(argc>2?atoi(argv[2]):100, argc>3?argv[3]:"");
  else if(cmd=="bench") bench();
  else cerr<<"usage: engine search <prefix> | engine plan <budget> <cat1,cat2>\n";
}
