#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "table.h"

static void enterLiveMap(G_table t, G_node flowNode, Temp_tempList temps){
	G_enter(t, flowNode, temps);
}

static Temp_tempList lookupLiveMap(G_table t, G_node flownode){
	return (Temp_tempList) G_look(t, flownode);
}

static G_node FindorCreateTempNode(G_graph graph, Temp_temp temp){
	G_nodeList nodeList = G_nodes(graph);
	while(nodeList){
		Temp_temp info = Live_gtemp(nodeList->head);
		if(info == temp){
			return nodeList->head;
		}
		nodeList = nodeList->tail;
	}
	//don't find a node, so create a new node
	return G_Node(graph, (void*)temp);
}

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}

Temp_temp Live_gtemp(G_node n) {
	//your code here.
	return (Temp_temp) G_nodeInfo(n);
}

struct Live_graph Live_liveness(G_graph flow) {
	G_table livemap = G_empty();
	G_table defsmap = G_empty();
	G_table usesmap = G_empty();
	G_table inmap = G_empty();
	G_table outmap = G_empty();

	G_nodeList flowNodeList = G_nodes(flow);
	G_nodeList reverseNodeList = NULL;
	///initialize all maps
	assert(flowNodeList);
	while(!flowNodeList){
		G_node flowNode = flowNodeList->head;
		reverseNodeList = G_NodeList(flowNodeList->head, reverseNodeList);
		//TODO:sort defList and useList if we use order-list 
		//DONE: I have sorted them.
		enterLiveMap(defsmap, flowNode, FG_def(flowNode));
		enterLiveMap(usesmap, flowNode, FG_use(flowNode));
		enterLiveMap(inmap, flowNode, NULL);
		enterLiveMap(outmap, flowNode, NULL);
	}

	//use multi-round iteration and build livemap
	bool stopIterFlag = FALSE;
	while(!stopIterFlag){
		stopIterFlag = TRUE;
		G_nodeList tempList = reverseNodeList;
		while(tempList){
			G_node node = tempList->head;
			Temp_tempList oldIn = lookupLiveMap(inmap, node);
			Temp_tempList oldOut = lookupLiveMap(outmap, node);
			Temp_tempList uses = lookupLiveMap(usesmap, node);
			Temp_tempList defs = lookupLiveMap(defsmap, node);
			Temp_tempList newIn = Temp_unionList(uses, Temp_exclusiveList(oldOut, defs));
			Temp_tempList newOut = NULL;
			G_nodeList succList = G_succ(node);
			while(succList){
				newOut = Temp_unionList(newOut, lookupLiveMap(inmap, succList->head));
			} 
			if(!Temp_isSameList(oldIn, newIn) || !Temp_isSameList(oldOut, newOut)){
				stopIterFlag = FALSE;
			}
			enterLiveMap(inmap, node, newIn);
			enterLiveMap(outmap, node, newOut);
			tempList = tempList->tail;
		}
	}

	struct Live_graph lg;
	G_graph conflictGraph = G_empty();
	
	//add machine registers first
	Temp_tempList registers = F_registers();
	while(registers){
		FindorCreateTempNode(conflictGraph, registers->head);
		registers = registers->tail;
	}

	//build conflict graph.
	Live_moveList moveList = NULL;
	flowNodeList = G_nodes(flow);
	G_nodeList tempFlowList = flowNodeList;
	while(!tempFlowList){
		G_node flowNode = tempFlowList->head;
		Temp_tempList defs = lookupLiveMap(defsmap, flowNode);

		//add movelist
		if(FG_isMove(flowNode)){
			//should only have one use and one def
			Temp_tempList uses = lookupLiveMap(usesmap, flowNode);
			assert(defs && !defs->tail);
			assert(uses && !uses->tail);
			
			G_node useNode = FindorCreateTempNode(conflictGraph, uses->head);
			G_node defNode = FindorCreateTempNode(conflictGraph, defs->head);
			moveList = Live_MoveList(useNode, defNode, moveList);

			//add conflictGraph
			Temp_tempList liveList = lookupLiveMap(outmap, flowNode);
			Temp_tempList tempDefs = defs;
			while(tempDefs){
				Temp_tempList tempLives = liveList;
				while(tempLives){ 
					// in MOVE instruction, we don't add a conflict edge between uses and defs
					if(containsTemp(uses, tempLives->head)){
						continue;
					}
					G_node liveNode = FindorCreateTempNode(conflictGraph, tempLives->head);
					G_addEdge(defNode, liveNode);
					tempLives = tempLives->tail;
				}
				tempDefs = tempDefs->tail;
			}
		}else{
			//add conflictGraph
			Temp_tempList liveList = lookupLiveMap(outmap, flowNode);
			Temp_tempList tempDefs = defs;
			while(tempDefs){
				Temp_tempList tempLives = liveList;
				G_node defNode = FindorCreateTempNode(conflictGraph, tempDefs->head);
				while(tempLives){ 
					G_node liveNode = FindorCreateTempNode(conflictGraph, tempLives->head);
					G_addEdge(defNode, liveNode);
					tempLives = tempLives->tail;
				}
				tempDefs = tempDefs->tail;
			}
		}
	}
	lg.graph = conflictGraph;
	lg.moves = moveList;
	return lg;
}


bool Move_inList(Live_moveList list, Temp_temp src, Temp_temp dst){
    while(list){
        if(list->src == src && list->dst == dst){
            return TRUE;
        }
        list = list->tail;
    }
    return FALSE;
}

//list1 U list2
Live_moveList Move_unionList(Live_moveList list1, Live_moveList list2){
    Live_moveList result = list1;
    while(list2){
        if(!Move_inList(list1, list2->src, list2->dst)){
            result = Live_MoveList(list2->src, list2->dst, result);
        }
        list2 = list2->tail;
    }
    return result;
}

//list1 ^ list2
Live_moveList Move_intersectList(Live_moveList list1, Live_moveList list2){
    Live_moveList result = NULL;
    while(list2){
        if(Move_inList(list1, list2->src, list2->dst)){
            result = Live_MoveList(list2->src, list2->dst, result);
        }
        list2 = list2->tail;
    }
	return result;
}

//list1 - list2
Live_moveList Move_exclusiveList(Live_moveList list1, Live_moveList list2){
    Live_moveList result = NULL;
    while(list1){
        if(!Move_inList(list2, list1->src, list1->dst)){
            result = Live_MoveList(list1->src, list1->dst, result);
        }
        list1 = list1->tail;
    }
    return result;
}

bool Move_isSameList(Live_moveList list1, Live_moveList list2){
  return (Move_exclusiveList(list1, list2) == NULL && Move_exclusiveList(list2, list1) == NULL);
}

Live_moveList Move_insertMove(Live_moveList list, Temp_temp src, Temp_temp dst){
	return Live_MoveList(src, dst, list);
}

Live_moveList Move_deleteMove(Live_moveList list, Temp_temp src, Temp_temp dst){
	if(!list) return list;
    if(list->src == src && list->dst == dst){
      return list->tail;
    }
    list->tail = Move_deleteMove(list->tail, src, dst);
    return list;
}


