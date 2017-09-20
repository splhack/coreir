#include "op_graph.hpp"

#include "algorithm.hpp"
#include "utils.hpp"

using namespace std;

namespace CoreIR {

  Wireable* extractSource(Select* sel) {
    Wireable* p = sel->getParent();

    // Every select off of self gets its own node
    if (fromSelf(sel) && (!isSelect(p))) {
      return sel;
    }

    // Base case for non self connections.
    if (!isSelect(p)) {
      return p;
    }

    return extractSource(toSelect(p));
  }
  
  WireNode getNode(const NGraph& g, const vdisc vd) {
    return g.getNode(vd);
  }

  Conn getConn(const NGraph& g, const edisc ed) {
    return g.getConn(ed);
  }

  std::vector<Conn> getInputConnections(const vdisc vd, const NGraph& g) {
    return g.getInputConnections(vd);
  }

  std::vector<Conn> NGraph::getInputConnections(const vdisc vd) const {
    vector<Conn> inConss;

    Wireable* w = getNode(vd).getWire();

    for (auto out_edge_desc : inEdges(vd)) {

      Conn edge_conn =
	getConn(out_edge_desc);

      assert(isSelect(edge_conn.second.getWire()));

      CoreIR::Select* sel = static_cast<Select*>(edge_conn.second.getWire());

      assert(extractSource(sel) == w);

      inConss.push_back(edge_conn);
    }
  
    return inConss;

  }

  std::vector<Wireable*> NGraph::getOutputs(const vdisc vd) const {
    vector<Wireable*> outputs;

    Wireable* w = getNode(vd).getWire();

    for (auto out_edge_desc : outEdges(vd)) {

      Conn edge_conn =
	getConn(out_edge_desc);

      assert(isSelect(edge_conn.first.getWire()));
      Select* sel = static_cast<Select*>(edge_conn.first.getWire());
      assert(sel->getParent() == w);

      outputs.push_back(edge_conn.second.getWire());
      
    }

    return outputs;
  }
  
  std::vector<Wireable*> getOutputs(const vdisc vd, const NGraph& g) {
    return g.getOutputs(vd);
  }

  std::vector<Wireable*> NGraph::getInputs(const vdisc vd) const {
    vector<Wireable*> inputs;
    Wireable* w = getNode(vd).getWire();

    for (auto in_edge_desc : inEdges(vd)) {

      Conn edge_conn =
	getConn(in_edge_desc);

      assert(isSelect(edge_conn.second.getWire()));

      Select* sel = static_cast<Select*>(edge_conn.second.getWire());

      assert(extractSource(sel) == w); //->getParent() == w);

      inputs.push_back(edge_conn.first.getWire());
      
    }

    return inputs;

  }

  std::vector<Wireable*> getInputs(const vdisc vd, const NGraph& g) {
    return g.getInputs(vd);
  }

  vector<vdisc> vertsWithNoIncomingEdge(const NGraph& g) {
    return g.vertsWithNoIncomingEdge();
  }

  vector<vdisc> NGraph::vertsWithNoIncomingEdge() const {
    vector<vdisc> vs;


    for (auto v : getVerts()) {

      if (getInputConnections(v).size() == 0) {
	vs.push_back(v);
      }
    }

    return vs;
    
  }

  int numVertices(const NGraph& g) {
    return g.numVertices();
  }

  int NGraph::numVertices() const {

    return verts.size();

  }

  std::deque<vdisc> topologicalSort(const NGraph& g) {
    deque<vdisc> topo_order;

    vector<vdisc> s = vertsWithNoIncomingEdge(g);

    //vector<edisc> deleted_edges;
    unordered_set<edisc> deleted_edges;

    while (s.size() > 0) {
      vdisc vd = s.back();
      topo_order.push_back(vd);
      s.pop_back();

      
      for (auto ed : g.outEdges(vd)) {

	//deleted_edges.push_back(ed);
	deleted_edges.insert(ed);
	
	vdisc src = g.source(ed);
	vdisc dest = g.target(ed);

	assert(src == vd);

	bool noOtherEdges = true;

	for (auto in_ed : g.inEdges(dest)) {

	  if (!elem(in_ed, deleted_edges)) {
	    noOtherEdges = false;
	    break;
	  }
	}

	if (noOtherEdges){
	  s.push_back(dest);
	}
      }


    }

    assert(topo_order.size() == numVertices(g));

    return topo_order;
  }

  std::vector<Conn> getOutputConnections(const vdisc vd, const NGraph& g) {
    return g.getOutputConnections(vd);
  }

  std::vector<Conn> NGraph::getOutputConnections(const vdisc vd) const {
    vector<Conn> outConns;

    Wireable* w = getNode(vd).getWire();

    for (auto out_edge_desc : outEdges(vd)) {

      Conn edge_conn =
	getConn(out_edge_desc);

      assert(isSelect(edge_conn.first.getWire()));

      Select* sel = static_cast<Select*>(edge_conn.first.getWire());

      assert(extractSource(sel) == w);
      //assert(sel->getParent() == w);

      outConns.push_back(edge_conn);
      
    }
  
    return outConns;
  }


  void addConnection(unordered_map<WireNode, vdisc>& imap,
		     Conn& conn,
		     NGraph& g) {

    assert(isSelect(conn.first.getWire()));
    assert(isSelect(conn.second.getWire()));

    auto c1 = static_cast<Select*>(conn.first.getWire());
    auto c2 = static_cast<Select*>(conn.second.getWire());

    Wireable* p1 = extractSource(c1);

    vdisc c1_disc;
    if (isRegisterInstance(p1)) {
      auto c1_disc_it = imap.find(outputNode(p1));

      assert(c1_disc_it != imap.end());

      c1_disc = (*c1_disc_it).second;

    } else {
      assert(!isRegisterInstance(p1));

      auto c1_disc_it = imap.find(combNode(p1));

      assert(c1_disc_it != imap.end());

      c1_disc = (*c1_disc_it).second;
    }
      
    Wireable* p2 = extractSource(c2);

    vdisc c2_disc;
    if (isRegisterInstance(p2)) {
      auto c2_disc_it = imap.find(receiverNode(p2));

      assert(c2_disc_it != imap.end());

      c2_disc = (*c2_disc_it).second;
    } else {
      assert(!isRegisterInstance(p2));

      auto c2_disc_it = imap.find(combNode(p2));

      assert(c2_disc_it != imap.end());

      c2_disc = (*c2_disc_it).second;
    }
      
    edisc ed = g.addEdge(c1_disc, c2_disc);

    g.addEdgeLabel(ed, conn);
    
  }

  void addWireableToGraph(Wireable* w1,
			  unordered_map<WireNode, vdisc>& imap,
			  NGraph& g) {

    if (isInstance(w1)) {
      Instance* inst = toInstance(w1);
      string genRefName = getInstanceName(*inst);

      if (genRefName == "reg") {
	WireNode wOutput = outputNode(w1);//{w1, true, false};
	WireNode wInput = receiverNode(w1); //{w1, true, true};

	if (imap.find(wOutput) == end(imap)) {
	  cout << "Adding register output" << endl;
	  auto v1 = g.addVertex(wOutput);
	  imap.insert({wOutput, v1});
	}

	if (imap.find(wInput) == end(imap)) {
	  cout << "Adding register input" << endl;
	  auto v1 = g.addVertex(wInput);
	  imap.insert({wInput, v1});
	}

	return;
      }
    }

    if (imap.find(combNode(w1)) == end(imap)) {
      WireNode w = combNode(w1);

      vdisc v1 = g.addVertex(w);
      imap.insert({w, v1});
    }

  }

  vector<Conn> buildOrderedConnections(Module* mod) {
    vector<Conn> conns;

    assert(mod->hasDef());

    for (auto& connection : mod->getDef()->getConnections()) {

      assert(connectionIsOrdered(connection));


      Wireable* fst = connection.first;
      Wireable* snd = connection.second;

      assert(isSelect(fst));
      assert(isSelect(snd));

      Wireable* fst_p = toSelect(*fst).getParent();
      Wireable* snd_p = toSelect(*snd).getParent();

      Select* fst_select = static_cast<Select*>(fst);

      Type* fst_tp = fst_select->getType();

      InstanceValue w_fst(toSelect(fst)); //, false, false};
      InstanceValue w_snd(toSelect(snd)); //, {snd, false, false};

      if (fst_tp->isInput()) {

	// if (isRegisterInstance(fst_p)) {
	//   w_fst = {fst, true, true};
	// }

	// if (isRegisterInstance(snd_p)) {
	//   w_snd = {snd, true, false};
	// }

	
	conns.push_back({w_snd, w_fst});
      } else {

	// if (isRegisterInstance(fst_p)) {
	//   w_fst = {fst, true, false};
	// }

	// if (isRegisterInstance(snd_p)) {
	//   w_snd = {snd, true, true};
	// }

	conns.push_back({w_fst, w_snd});
      }

    }

    assert(conns.size() == mod->getDef()->getConnections().size());

    return conns;
  
  }

  void buildOrderedGraph(Module* mod, NGraph& g) {

    //cout << "Building ordered conns" << endl;

    auto ord_conns = buildOrderedConnections(mod);

    // Add vertexes for all instances in the graph
    unordered_map<WireNode, vdisc> imap;

    for (auto& conn : ord_conns) {

      Select* sel1 = toSelect(conn.first.getWire());
      Select* sel2 = toSelect(conn.second.getWire());

      Wireable* w1 = extractSource(sel1);
      Wireable* w2 = extractSource(sel2);

      addWireableToGraph(w1, imap, g);
      addWireableToGraph(w2, imap, g);

    }

    // Add edges to the graph
    for (Conn conn : ord_conns) {
      addConnection(imap, conn, g);
    }

  }

  InstanceValue findArg(string argName, std::vector<Conn>& ins) {
    for (auto& conn : ins) {
      InstanceValue arg = conn.first;
      InstanceValue placement = conn.second;
      string selName = toSelect(placement.getWire())->getSelStr();
      if (selName == argName) {
	return arg;
      }
    }

    cout << "Error: Could not find argument: " << argName << endl;

    assert(false);
  }

  void eliminateMasks(const std::deque<vdisc>& topoOrder,
		      const NGraph& g) {
    
  }

  int numMasksNeeded(const NGraph& g) {
    int numMasks = 0;

    for (auto& vd : g.getVerts()) {

      for (auto& conn : g.getOutputConnections(vd)) {

	// WireNode& in = conn.first;
	// WireNode& out = conn.second;

	// WireNode sourceNode = g.getSourceNode(in);

	// if (g.getInptuConnectsion(sourceNode)) {
	  
	// }

	if (g.getNode(vd).highBitsAreDirty()) {
	  numMasks++;
	}
      }

    }

    return numMasks;
  }

}
