/*
 * Equivalence Checker for netlists
 *
 *
 * Name 1: Saman Razzaghi
 * Matriculation Number 1: 861672
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>

using namespace std;

typedef enum
{
	AND,
	OR,
	INV,
	XOR
} GateType;

typedef struct
{
	GateType type;
	vector<int> nets;
} Gate;

typedef vector<Gate> GateList;

 // The following global variables are provided and automatically pre-filled (examples are for file xor2.net):

// Total number of nets in netlist 1 / 2
// E.g. netCount1 is 3
int netCount1, netCount2;

// Names of inputs / outputs in netlist 1 / 2
// E.g. inputs1[0] contains "a_0"
vector<string> inputs1, outputs1, inputs2, outputs2;

// Mapping from input / output names to net numbers in netlist 1 / 2
// E.g. map1["a_0"] is 1, map1["b_0"] is 2, ...
map<string, int> map1, map2;

// List (std::vector<Gate>) of all gates in netlist 1 / 2
// E.g.:
// - gates1[0].type is XOR
// - gates1[0].nets is std::vector<int> and contains three ints (one for each XOR port)
// - gates1[0].nets[0] is 1 (first XOR input port)
// - gates1[0].nets[1] is 2 (second XOR input port)
// - gates1[0].nets[2] is 3 (XOR output port)
GateList gates1, gates2;


int readFile(string filename, int & netCount, vector<string> & inputs, vector<string> & outputs, map<string, int> & map, GateList & gates)
{
	ifstream file(filename.c_str());
	if (! file.is_open())
	{
		return -1;
	}
	string curLine;
	// net count
	getline(file, curLine);
	netCount = atoi(curLine.c_str());
	// inputs
	getline(file, curLine);
	stringstream inputsStream(curLine);
	string buf;
	while (inputsStream >> buf)
	{
		inputs.push_back(buf);
	}
	// outputs
	getline(file, curLine);
	stringstream outputsStream(curLine);
	while (outputsStream >> buf)
	{
		outputs.push_back(buf);
	}
	// mapping
	for (size_t i=0; i<inputs1.size() + outputs1.size(); i++)
	{
		getline(file, curLine);
		stringstream mappingStream(curLine);
		mappingStream >> buf;
		int curNet = atoi(buf.c_str());
		mappingStream >> buf;
		map[buf] = curNet;
	}
	// empty line
	getline(file, curLine);
	if (curLine.length() > 1)
	{
		return -1;
	}
	// gates
	while (getline(file, curLine))
	{
		stringstream gateStream(curLine);
		gateStream >> buf;
		Gate curGate;
		if (buf != "and" && buf != "or" && buf != "inv" && buf != "xor")
		{
			cerr << "Unknown gate type: " << buf << endl;
			return -1;
		}
		curGate.type = (buf == "and" ? AND : buf == "or" ? OR : buf == "inv" ? INV : XOR);
		while (gateStream >> buf)
		{
			int curNet = atoi(buf.c_str());
			curGate.nets.push_back(curNet);
		}
		gates.push_back(curGate);
	}
	return 0;
}

int readFiles(string filename1, string filename2)
{
	if (readFile(filename1, netCount1, inputs1, outputs1, map1, gates1) != 0)
	{
		return -1;
	}
	if (readFile(filename2, netCount2, inputs2, outputs2, map2, gates2) != 0)
	{
		return -1;
	}
	return 0;
}


// BDD Node Definition
struct BDDNode {
    int var;
    BDDNode* low;
    BDDNode* high;

    BDDNode(int v, BDDNode* l, BDDNode* h) : var(v), low(l), high(h) {}
    bool operator==(const BDDNode& other) const {
        return var == other.var && low == other.low && high == other.high;
    }
};

// Terminal Nodes
BDDNode* BDD_TRUE = new BDDNode(-1, nullptr, nullptr);
BDDNode* BDD_FALSE = new BDDNode(-2, nullptr, nullptr);

// Global node cache to avoid duplicates
map<tuple<int, BDDNode*, BDDNode*>, BDDNode*> node_cache;

// BDD creation (with cache)
BDDNode* make_bdd(int var, BDDNode* low, BDDNode* high) {
    if (low == high) return low;
    auto key = make_tuple(var, low, high);
    if (node_cache.count(key)) return node_cache[key];
    BDDNode* node = new BDDNode(var, low, high);
    node_cache[key] = node;
    return node;
}

// Cofactor computation
pair<BDDNode*, BDDNode*> cofactor(BDDNode* f, int x) {
    if (f == BDD_TRUE || f == BDD_FALSE || f->var > x) {
        return {f, f};
    }
    if (f->var == x) {
        return {f->low, f->high};
    }
    auto [f0_low, f0_high] = cofactor(f->low, x);
    auto [f1_low, f1_high] = cofactor(f->high, x);
    return {
        make_bdd(f->var, f0_low, f1_low),
        make_bdd(f->var, f0_high, f1_high)
    };
}

// ITE function (core of BDD construction)
BDDNode* ite(BDDNode* f, BDDNode* g, BDDNode* h) {
    if (f == BDD_TRUE) return g;
    if (f == BDD_FALSE) return h;
    if (g == h) return g;
    if (g == BDD_TRUE && h == BDD_FALSE) return f;

    int top = std::max(f->var, std::max(g->var, h->var));

    auto [f0, f1] = cofactor(f, top);
    auto [g0, g1] = cofactor(g, top);
    auto [h0, h1] = cofactor(h, top);
    return make_bdd(top, ite(f0, g0, h0), ite(f1, g1, h1));
}

// Apply logic gate using ITE
BDDNode* apply_gate(GateType type, BDDNode* a, BDDNode* b = nullptr) {
    switch (type) {
        case AND:
            return ite(a, b, BDD_FALSE);
        case OR:
            return ite(a, BDD_TRUE, b);
        case INV:
            return ite(a, BDD_FALSE, BDD_TRUE);
        case XOR:
            return ite(a, ite(b, BDD_FALSE, BDD_TRUE), b);
        default:
            cerr << "Unknown gate type\n";
            exit(1);
    }
}

// Build BDD from a gate list and input/output map
void build_bdd(const vector<string>& inputs,
               const map<string, int>& net_map,
               const GateList& gates,
               map<int, BDDNode*>& cache,
               int offset = 0) {
    for (const string& in : inputs) {
        int net = net_map.at(in) + offset;
        cache[net] = make_bdd(net, BDD_FALSE, BDD_TRUE);
    }

    for (const Gate& gate : gates) {
        if (gate.type == INV) {
            int a = gate.nets[0] + offset;
            int out = gate.nets[1] + offset;
            cache[out] = apply_gate(INV, cache[a]);
        } else {
            int a = gate.nets[0] + offset;
            int b = gate.nets[1] + offset;
            int out = gate.nets[2] + offset;
            cache[out] = apply_gate(gate.type, cache[a], cache[b]);
        }
    }
}

// Check if BDD outputs are equivalent
bool are_equivalent(const vector<string>& outputs,
                    const map<string, int>& map1,
                    const map<string, int>& map2,
                    const map<int, BDDNode*>& cache1,
                    const map<int, BDDNode*>& cache2) {
    for (const string& out : outputs) {
        int net1 = map1.at(out);
        int net2 = map2.at(out);
        if (cache1.at(net1) != cache2.at(net2)) {
            return false;
        }
    }
    return true;
}



int main(int argc, char ** argv)
{
	if (argc != 3)
	{
		cerr << "Wrong argument count!\n";
		return -1;
	}
	if (readFiles(argv[1], argv[2]) != 0)
	{
		cerr << "Error while reading files!\n";
		return -1;
	}

	/*
	 * The following global variables are now defined (see above for data types and details):
	 * - netCount1, netCount2
	 * - inputs1, outputs1, inputs2, outputs2
	 * - map1, map2
	 * - gates1, gates2
	 */

  // Offset mapping2 and gates2 nets to avoid collisions
    for (auto& [name, net] : map2) net += 1000;
    for (Gate& g : gates2) for (int& n : g.nets) n += 1000;

    // Remap common inputs to same net numbers
    for (const string& name : inputs1) {
        if (map2.count(name)) {
            int net1 = map1[name];
            int net2 = map2[name];

            for (Gate& g : gates2) {
                for (int& n : g.nets) {
                    if (n == net2) n = net1;
                }
            }
            map2[name] = net1;
        }
    }

    // BDD caches per circuit
    map<int, BDDNode*> cache1, cache2;

    build_bdd(inputs1, map1, gates1, cache1, 0);
    build_bdd(inputs2, map2, gates2, cache2, 0);

    bool eq = are_equivalent(outputs1, map1, map2, cache1, cache2);
    cout << (eq ? "Equivalent" : "Not Equivalent") << endl;


	return 0;
}
