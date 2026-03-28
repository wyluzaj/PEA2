#include "bnb_plain_solver.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>
#include "bnb_node.h"
#include "ub.h"
#include "../common/utils.h"
namespace {
    constexpr int INF=std::numeric_limits<int>::max()/4;
    std::vector<int> materializePath(const std::vector<BnBNode>& pool,int id){
        std::vector<int> r; while(id!=-1){r.push_back(pool[id].current_vertex);id=pool[id].parent_index;}
        std::reverse(r.begin(),r.end()); return r;
    }
    BnBNode createRoot(const TSPInstance& inst){
        BnBNode r; r.visited.assign(inst.dimension,false); r.visited[Config::START_VERTEX]=true;
        r.parent_index=-1;r.current_vertex=Config::START_VERTEX;r.level=1;r.partial_cost=0;r.lower_bound=0;
        return r;
    }
    BnBNode createChild(const TSPInstance& inst,const BnBNode& par,int parIdx,int next){
        BnBNode c=par; c.parent_index=parIdx; c.visited[next]=true;
        c.partial_cost+=inst.distanceMatrix[par.current_vertex][next];
        c.current_vertex=next;c.level=par.level+1;c.lower_bound=c.partial_cost;
        return c;
    }
    bool timeLimitReached(const std::chrono::high_resolution_clock::time_point& s,double max){
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-s).count()>=max;
    }
    int completionCost(const TSPInstance& inst,const BnBNode& n){
        return n.partial_cost+inst.distanceMatrix[n.current_vertex][Config::START_VERTEX];
    }
    void initResult(TSPResult& r,const TSPInstance& inst,const std::string& name){
        r.algorithm_name=name;
        r.instance_name=inst.name.empty()?"unknown_instance":inst.name;
        r.instance_type=inst.type.empty()?(inst.symmetric?"TSP":"ATSP"):inst.type;
        r.vertex_count=inst.dimension; r.best_cost=INF;
        r.best_path.clear();r.best_path_text.clear();r.ub_from_nn=-1;
        r.visited_nodes=r.pruned_nodes=r.generated_nodes=r.stored_nodes=r.max_frontier_size=r.max_node_pool_size=0;
        r.memory_exhausted=false;r.completed_naturally=true;r.stop_reason="Zakonczono naturalnie";r.summary_file_name.clear();
    }
}
TSPResult solveBranchAndBoundPlain(const TSPInstance& inst,ISearchFrontier& frontier,
                                   const std::string& name,InitialUBMode ubMode,double maxTime){
    using clock=std::chrono::high_resolution_clock;
    if(!inst.isValid()) throw std::runtime_error("Instancja jest niepoprawna.");
    if(inst.dimension<2) throw std::runtime_error("Instancja musi miec co najmniej 2 miasta.");
    TSPResult result; initResult(result,inst,name);
    if(ubMode==InitialUBMode::RNN){
        auto ub=computeInitialUpperBoundRNN(inst); result.ub_from_nn=ub.cost;
        if(ub.path.size()==static_cast<size_t>(inst.dimension)){result.best_cost=ub.cost;result.best_path=ub.path;}
        else result.best_cost=ub.cost;
    }
    const auto start=clock::now();
    try {
        std::vector<BnBNode> pool; pool.reserve(100000);
        pool.push_back(createRoot(inst));
        result.stored_nodes=1; result.max_node_pool_size=1;
        frontier.push(0,pool[0].partial_cost);
        result.max_frontier_size=std::max(result.max_frontier_size,(long long)frontier.size());
        while(!frontier.empty()){
            if(timeLimitReached(start,maxTime)){result.completed_naturally=false;result.stop_reason="Przekroczono limit czasu";break;}
            result.max_frontier_size=std::max(result.max_frontier_size,(long long)frontier.size());
            result.max_node_pool_size=std::max(result.max_node_pool_size,(long long)pool.size());
            const NodeId id=frontier.pop(); const BnBNode node=pool[id]; result.visited_nodes++;
            if(node.partial_cost>=result.best_cost){result.pruned_nodes++;continue;}
            if(node.level==inst.dimension){
                const int fc=completionCost(inst,node);
                if(fc<result.best_cost){result.best_cost=fc;result.best_path=materializePath(pool,id);}
                continue;
            }
            for(int next=0;next<inst.dimension;++next){
                if(node.visited[next]) continue;
                result.generated_nodes++;
                BnBNode child=createChild(inst,node,id,next);
                if(child.partial_cost<result.best_cost){
                    pool.push_back(std::move(child)); result.stored_nodes++;
                    result.max_node_pool_size=std::max(result.max_node_pool_size,(long long)pool.size());
                    NodeId cid=(NodeId)(pool.size()-1);
                    frontier.push(cid,pool[cid].partial_cost);
                    result.max_frontier_size=std::max(result.max_frontier_size,(long long)frontier.size());
                } else result.pruned_nodes++;
            }
        }
    } catch(const std::bad_alloc&){result.completed_naturally=false;result.memory_exhausted=true;result.stop_reason="Brak pamieci";}
    result.total_time_ms=elapsedMilliseconds(start,clock::now());
    if(!result.best_path.empty()&&result.best_path.size()==static_cast<size_t>(inst.dimension))
        result.best_path_text=pathToString(result.best_path,inst);
    else{result.best_cost=-1;result.best_path_text="Brak pelnej sciezki";}
    return result;
}
