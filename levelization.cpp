#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <bits/stdc++.h>

using namespace std;

// A struct to represent a transistor
struct Transistor {
    string name;
    string type;
    string source;
    string gate;
    string drain;
    string status; // none, open, closed

    Transistor() {
        name = "";
        type = "";
        source = "";
        gate = "";
        drain = "";
        status = "none";
    }
};

// A struct to represent a wire
struct Wire {
    string t1;
    string t2;
    string type ; // source, drain, gate
    int level;
    string voltage;

    Wire() {
        t1 = " ";
        t2 = " ";
        type = " ";
        level = -1; // begins unknown
        voltage = "-1"; // begins unknown
    }
};



// Function to parse a netlist
void parse(vector<Transistor*>& transistors, vector<Wire*>& wires, string filename) {
    // Save each line
    string line;
    ifstream myfile(filename);

    while(getline(myfile, line)) {
        // Iterate through each word in the line
        stringstream myline(line);
        Transistor* t = new Transistor();

        myline >> t->name;
        myline >> t->source;
        myline >> t->gate;
        myline >> t->drain;
        myline >> t->type; // not used
        myline >> t->type;

        // Add transistor to the set
        transistors.push_back(t);

        // Add wires, initially setting all levels to -1 (unknown)
        Wire* w1 = new Wire;
        w1->t1 = t->name;
        w1->t2 = t->source;
        w1->type = "source";
        wires.push_back(w1);

        Wire* w2 = new Wire;
        w2->t1 = t->name;
        w2->t2 = t->gate;
        w2->type = "gate";
        wires.push_back(w2);

        Wire* w3 = new Wire;
        w3->t1 = t->name;
        w3->t2 = t->drain;
        w3->type = "drain";
        wires.push_back(w3);

    }
}



// Function for levelization algorithm
bool levelize(vector<Wire*>& wires, int& maxLevel) {
    maxLevel = 0;

    // Levelization algorithm

    // Set all wires with VDD, GND, or IN to level 0
    vector<Wire*> levelized;
    for(Wire* w : wires) {
        if(w->t2.substr(0,3) == "VDD" || w->t2.substr(0,3) == "GND" || w->t2.substr(0,2) == "IN") {
            w->level = 0;
            // Add these wires to levelized list
            levelized.push_back(w);
        }
    }

    // Explore each wire in levelized list
    int count = 0;
    for(int i = 0; i < levelized.size(); i++) {
        count++;
        if (count > 1000) return false;

        Wire* w = levelized[i];

        // Create a set of neighbor wires
        vector<Wire*> neighbors;
        // Iterate through all wires, adding the undiscovered neighbors
        for(Wire* w2 : wires) {
            if(w2->level == 0) continue;
            // If current wire is a gate
            if(w->type == "gate") {
                // Add neighbor if it's a connected source/drain whose level isn't 0
                if(w->t1 == w2->t1 && w->t2 != w2->t2 && w2->level != 0) neighbors.push_back(w2);
                // Add neighbor if it's connected directly but it's level is lower
                if(w->t1 != w2->t1 && w->t2 == w2->t2 && w2->level <= w->level) neighbors.push_back(w2);
                // Add neighbor if same t1/t2 but different types
                if(w->t1 == w2->t1 && w->t2 == w2->t2 && w2->type != w->type) neighbors.push_back(w2);
            }
            // If current wire is a source or drain            
            else {
                // Add neighboring wire if it has a different level
                if(w->t1 == w2->t1 && w->t2 != w2->t2 && w->level != w2->level) neighbors.push_back(w2);
                if(w->t1 != w2->t1 && w->t2 == w2->t2 && w->level != w2->level) neighbors.push_back(w2);
            }
        }

        /*
        cout << "Current wire: " << w->t1 << " " << w->t2 << "\tType: " << w->type << endl;
        cout << "Neighbors: ";
        for(Wire* w : neighbors) cout << w->t1 << " " << w->t2 << "\t";
        cout << endl;*/


        // Iterate through each neighbor
        for(Wire* n : neighbors) {

            // If second part of wire is same, that means level should be same
            if(w->t2 == n->t2) {
                // If transistor is same but gate is different, then levelization unsuccessful
                if(w->t1 == n->t1 && (w->type == "gate" || n->type == "gate")) return false;


                if(n->level < w->level) n->level = w->level;
                else w->level = n->level;
                // Add n to levelized list
                levelized.push_back(n);
                continue;
            }

            // Check if current wire w can affect its neighbor n
            if(w->type == "source" || w->type == "drain") {
                // Source/drain can't affect gate
                if(n->type == "gate") continue;

                // Level of source = level of drain, unless current wire's level is 0
                if(w->level != 0 && n->level != 0) {
                    n->level = w->level;
                    // Add n to levelized list
                    levelized.push_back(n);
                }
            }
            // Else, type is gate
            else {
                // Gate can affect source and drain

                // Exception is level 0
                if(n->level != 0) {
                    n->level = w->level + 1;
                    // Add n to levelized list
                    levelized.push_back(n);
                }          
            }
        }

        // Check if max level changed
        if(w->level > maxLevel) maxLevel = w->level;
    }

    return true;
}

// Creates input combinations
// Ex: 00 01 10 11
void createInputs(int size, string c, vector<string>& result) {
    if(size == 0) {
        result.push_back(c);
        return;
    }

    createInputs(size-1, c+"0", result);
    createInputs(size-1, c+"1", result);
}


// Sorts wires list by level
void sort(vector<Wire*>& wires) {
    for(int i = 0; i < wires.size(); i++) {
        for(int j = i; j < wires.size(); j++) {
            if(wires[j]->level < wires[i]->level) {
                Wire* temp = wires[j];
                wires[j] = wires[i];
                wires[i] = temp;
            }
        }
    }
}


// Rearranges a wire in a vector of wires
void rearrange(vector<Wire*>& v) {
    Wire* w = v.back();
    for(int i = v.size() - 2; i >= 0; i--) {
        v[i+1] = v[i];
    }
    v[0] = w;
}

// Sorts a vector by putting wires with gates at the front
void sortGates(vector<Wire*>& v) {
    for(int i = v.size()-1; i > 0; i--) {
        for(int j = i; j >= 0; j--) {
            if(v[i]->type == "gate" && v[j]->type != "gate") {
                Wire* temp = v[i];
                v[i] = v[j];
                v[j] = temp;
            }
        }
    }
}

// Helper function to print output value
void printOutput(vector<Wire*>& v) {
    vector<Wire*> floatingOutputs;
    vector<Wire*> otherOutputs;
    for(Wire* w : v) {
        if(w->t2.substr(0,3) == "OUT") {
            if (w->voltage == "z") floatingOutputs.push_back(w);
            else otherOutputs.push_back(w);
        }
    }

    if(otherOutputs.size() == 0) {
        cout << "OUT: z" << endl;
        return;
    }

    for(int i = 1; i < otherOutputs.size(); i++) {
        if(otherOutputs[i-1]->voltage != otherOutputs[i]->voltage) {
            cout << "OUT: r" << endl;
            return;
        }
    }

    cout << "OUT: " << otherOutputs[0]->voltage << endl;

}


// Set voltages
void setVoltages(vector<Wire*>& wires, vector<Transistor*>& transistors, string combination, int maxLevel) {
    cout << endl << "Combination: " << combination << endl;

    // Sort wires with a map from level to a vector of wires
    map<int, vector<Wire*>> ordered;
    for(int i = 0; i <= maxLevel; i++) {
        vector<Wire*> v;
        // Add wires of level i to v
        for(Wire* w : wires) {
            if(w->level == i) v.push_back(w);
        }
        // Sort v to put gates at end
        sortGates(v);
        ordered[i] = v;
    }

    // Assign voltages in levelization order
    bool infiniteLoop;
    vector<Wire*> errors;
    for(int i = 0; i <= maxLevel; i++) {
        vector<Wire*>& v = ordered.find(i)->second;

        // Iterate until the list is empty (pop from list when voltage assigned)
        int count = 0;
        while(!v.empty()) {
            // Use back of list since easy to pop when voltage assigned
            int rearrangeCount = 0;
            Wire* w = v.back();

            if (++count > 1000) break;

            // Set VDD to 1
            if(w->t2.substr(0,3) == "VDD") {
                w->voltage = "1";
                //v.pop_back();
            }
            // Set GND to 0
            else if(w->t2.substr(0,3) == "GND") {
                w->voltage = "0";
                //v.pop_back();
            }
            // Set inputs to their respective voltages
            else if(w->t2.substr(0,2) == "IN") {
                // Set the inputNum to be the index of the combination string
                int inputNum = (w->t2.back() - '0') - 1;

                // Set voltage to be what the combination string states
                w->voltage = combination[inputNum];

                //v.pop_back();
            }
            // Else, voltage must be found from neighboring wires
            else {
                // Explore other wires
                for(int i = 0; i < wires.size(); i++) {
                    Wire* w2 = wires[i];

                    // If level gets higher than current wire, leave the loop
                    if(w2->level > w->level) break;

                    // If w2's voltage isn't known, continue
                    if(w2->voltage == "-1") continue;

                    // If w2 is a direct neighbor (connected wire), level must be same
                    bool possibleRatio = false;
                    if(w2->t2 == w->t2 && w2->voltage != "z") {
                        w->voltage = w2->voltage;
                        // Check for possible ratios
                        possibleRatio = true;
                        //v.pop_back();
                        //break;
                    }


                    // If w2 is part of the same transistor, level can be calculated
                    if(w->t2 != w2->t2 && w2->t1 == w->t1 && (w2->type == "source" || w2->type == "drain") && (w->type == "source" || w->type == "drain")) {
                        // Find transistor
                        Transistor* currentTransistor;
                        for(Transistor* t : transistors) {
                            if(t->name == w->t1) currentTransistor = t;
                        }

                        if(currentTransistor->status == "open") {
                            if(w->voltage != "1" && w->voltage != "0") w->voltage = "z";
                        }

                        if(currentTransistor->status == "closed") {
                            if(w->voltage == "-1" || w->voltage == "z") w->voltage = w2->voltage;
                            else {
                                // If voltages are different
                                if((w->voltage == "0" && w2->voltage == "1") || (w->voltage == "1" && w2->voltage == "0")) w->voltage = "r"; // ratio
                                if(possibleRatio && w2->voltage != w->voltage) w->voltage = "r";
                            }
                            //v.pop_back();
                            //break;
                        }
                    }

                    // If wire still not levelized, set it to be floating
                    if(w->voltage == "-1") w->voltage = "z";
                    //if(w->voltage != "z") cout << w->t1 << " " << w->t2 << "\t" << w->type << " set to voltage " << w->voltage << " with w2 " << w2->t1 << " " << w2->t2 << " " << w2->voltage << endl;
                }

            }

            // Pop wire
            if((w->voltage != "-1" && w->voltage != "z") || rearrangeCount > 20) {
                v.pop_back();
            } else {
                rearrange(v);
                rearrangeCount++;
            }


            // Update transistor statuses
            if(w->type == "gate" && (w->voltage == "1" || w->voltage == "0")) {
                // Find transistor
                Transistor* currentTransistor;
                for(Transistor* t : transistors) {
                    if(t->name == w->t1) {
                        currentTransistor = t;
                        break;
                    }
                }

                // Set status
                if(currentTransistor->type == "NMOS") {
                    if(w->voltage == "0") currentTransistor->status = "open";
                    else if(w->voltage == "1") currentTransistor->status = "closed";
                } else {
                    if(w->voltage == "0") currentTransistor->status = "closed";
                    else if(w->voltage == "1") currentTransistor->status = "open";
                }
            }

            // If w is a gate and voltage is ratio or floating, end simulation
            if(w->type == "gate" && (w->voltage == "z" || w->voltage == "r")) {
                bool printInfo = true;

                for(Wire* w2 : errors) {
                    if(w->t1 == w2->t1 && w->t2 == w2->t2 && w->voltage == w2->voltage) printInfo = false;
                }
                
                if(printInfo) cout << "Error: wire <" << w->t1 << " " << w->t2 << ", gate> has voltage type " << w->voltage << endl;
                
                //rearrange(v);
                errors.push_back(w);

                if(v.size() == 1) break;
            }

        }

    }

    // Check if any wires need to be changed from floating
    // Can be commented out to leave each wire's individual voltages separate
    /*
    for(Wire* w : wires) {
        if(w->voltage == "z") {
            for(Wire* w2 : wires) {
                if((w2->t2 == w->t2) && (w2->voltage == "1" || w2->voltage == "0")) {
                    w->voltage = w2->voltage;
                    break;
                }
            }
        }
    }

    // Output results
    for(Wire* w : wires) {
        cout << w->t1 << " " << w->t2 << " " << w->type << "\tset to voltage " << w->voltage << endl;
    }*/

    if(!infiniteLoop) printOutput(wires);

    cout << endl;
    for(Wire* w : wires) cout << "\t" << w->t1 << " " << w->t2 << "\t\t" << w->type << "\t\t has voltage " << w->voltage << endl;

    // Now, reset all statuses and voltages
    for(Wire* w : wires) w->voltage = "-1";
    for(Transistor* t : transistors) t->status = "none";
    ordered.clear();
}


// Simulate
void simulate(vector<Wire*>& wires, vector<Transistor*>& transistors, int maxLevel) {
    // First, find number of inputs
    set<string> ins;
    for(Wire* w : wires) {
        if(w->t2.substr(0,2) == "IN") {
            ins.insert(w->t2);
        }
    }

    // Create all combinations of inputs
    vector<string> inputs;
    createInputs(ins.size(), "", inputs);

    // Sort wires list itself
    sort(wires);

    // Loop through each combination
    for(string combination : inputs) {
        setVoltages(wires, transistors, combination, maxLevel);
    }
}


int main() {
    // Create lists for transistors and wires
    vector<Transistor*> transistors;
    vector<Wire*> wires;

    // Read in contents from the file
    string filename = "circuit2.in";
    parse(transistors, wires, filename);

    // Levelize the wires
    int maxLevel = 0;
    bool levelizeSuccess = levelize(wires, maxLevel);

    if(!levelizeSuccess) {
        cout << "Levelization failed." << endl;
        //exit(0);
    }

    cout << endl;
    for(Wire* w : wires) cout << "\t" << w->t1 << " " << w->t2 << "\t\t" << w->type << "\t\t has level " << w->level << endl;

    // Simulate
    simulate(wires, transistors, maxLevel);


    for(Transistor* t : transistors) delete t;
    for(Wire* w : wires) delete w;


    cout << endl;
    return 0;
}