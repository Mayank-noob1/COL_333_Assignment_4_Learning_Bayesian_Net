#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <map>
#include <set>


class Node{
    // Name
    int name;                       // f : () -> name
    int n;
    // n - arity
    std::vector<std::string> values;        // f : i -> name
    std::map<std::string,int> value_to_index;    // f : name -> i

    // Markov blanket - Parent(Child)
    std::vector<int> parents_order;
    std::map<int,int> parents;       // f : i -> parent
    std::set<int> children;      // f : i -> child
    std::set<int> markovBlanket;
    // CPT
    public:
    std::vector<int> weights;
    std::vector<double> CPT;
    Node(int i){
        this->name = i;
    }
    Node (int i,std::vector<std::string> values){
        this->name = i;
        this->values = values;
        this->n = values.size();
    }
    void addValue(std::string val){
        if (value_to_index.find(val) != value_to_index.end()){
            return;
        }
        values.push_back(val);
        value_to_index[val] = values.size()-1;
        n++;
    }
    void addChild(int child){
        children.insert(child);
    }
    void addParent(int parent){
        if (isParent(parent)){return;}
        parents[parent]= parents_order.size();
        parents_order.push_back(parent);
    }
    void setCPT(std::vector<double> &Cpt){
        CPT.clear();
        CPT = std::vector<double> (Cpt.begin(), Cpt.end());
    }
    void setParents(std::vector<int> &parent){
        for (int i=0;i<parent.size();i++){
            parents[parent[i]] = parents_order.size();
            markovBlanket.insert(parent[i]);
            parents_order.push_back(parent[i]);
        }
    }
    int getValToIndex(std::string val){
        if (value_to_index.find(val) == value_to_index.end()){
            return -1;
        }
        return value_to_index[val];
    }
    bool isParent(int parent){
        return parents.find(parent) != parents.end();
    }
    bool isChild(int child){
        return children.find(child) != children.end();
    }
    void addMarkov(int i){
        markovBlanket.insert(i);
    }
    std::set<int> getChildren(){
        return children;
    }
    int getName()
    {return name;}
    int getnVal(){
        return n;
    }
    std::vector<double>& getCPT(){return CPT;}
    void setWeight(){};

};

class Network{
   // Name
    std::string name;                       // f : () -> name

    // n - arity
    std::vector<std::string> node_name;        // f : i -> node_name
    std::vector<Node*> nodes;                    // f: i -> node *
    std::map<std::string,int> name_to_index;    // f : node_name -> i

    public:
    Network(std::string name){
        this->name = name;
    }
    Node* addNode(std::string name){
        if (name_to_index.find(name) != name_to_index.end()){
            return nullptr;
        }
        node_name.push_back(name);
        name_to_index[name] = node_name.size()-1;
        Node * node = new Node(node_name.size()-1);
        nodes.push_back(node);
        return node;
    }
    Node* addNode(std::string name,std::vector<std::string>&val){
        if (name_to_index.find(name) != name_to_index.end()){
            return nullptr;
        }
        node_name.push_back(name);
        name_to_index[name] = node_name.size()-1;
        Node * node = new Node(node_name.size()-1,val);
        nodes.push_back(node);
        return node;
    }
    Node* getNode(std::string name){
        if (name_to_index.find(name) == name_to_index.end()){
            return nullptr;
        }
        return nodes[name_to_index.find(name)->second];
    }
    Node* getNode(int i){
        if (i >= node_name.size()){
            return nullptr;
        }
        return nodes[i];
    }
    int getIndex(std::string name){
        if (name_to_index.find(name) == name_to_index.end()){
            return -1;
        }
        return name_to_index.find(name)->second;
    }
    int getVarCount(){
        return node_name.size();
    }
    void generateMarkovBlanket(){
        for (auto &node: nodes){
            for (auto &child: node->getChildren()){
                for(auto &parent: nodes){
                    if ((parent != node) && (parent->isChild(child))){
                        node->addMarkov(parent->getName());
                    }
                }
            }
        }
    }
};

Network readNet(std::string FileName){
    Network Net(FileName);
	std::string line;
  	std::ifstream myFile("Alarm.bif"); 
  	std::string temp;
  	std::string name;
  	std::vector<std::string> values;
    std::vector<int>parents;
  	
    if (myFile.is_open())
    {
    	while (!myFile.eof() )
    	{
    		std::stringstream ss;
      		getline (myFile,line);
      		ss.str(line);
     		ss>>temp;
     		
     		if(temp.compare("variable")==0)
     		{       
     				ss>>name;
     				getline (myFile,line);
     				std::stringstream ss_;
     				ss_.str(line);
     				for(int i=0;i<4;i++)
     				{
     					ss_>>temp;
     				}
     				values.clear();
     				while(temp.compare("};")!=0)
     				{
     					values.push_back(temp);
     					ss_>>temp;
    				}
     				Node* pos=Net.addNode(name,values);
     		}
     		else if(temp.compare("probability")==0)
     		{
     				ss>>temp;
     				ss>>temp;

     				Node* node=Net.getNode(temp);
                    int index=Net.getIndex(temp);
                    ss>>temp;
                    values.clear();
     				while(temp.compare(")")!=0)
     				{
                        Node* node_=Net.getNode(temp);
                        node_->addChild(index);
                        int index_ = Net.getIndex(temp);
                        if (index_ == -1){
                            Net.addNode(temp);
                        }
                        index_ = Net.getIndex(temp);
     					parents.push_back(index_);

     					ss>>temp;
    				}
                    node->setParents(parents);
    				getline (myFile,line);
     				std::stringstream ss_;
                    
     				ss_.str(line);
     				ss_>> temp;
     				ss_>> temp;
                    
     				std::vector<double> curr_CPT;
                    std::string::size_type sz;
     				while(temp.compare(";")!=0)
     				{
     					curr_CPT.push_back(atof(temp.c_str()));
     					ss_>>temp;
    				}
                    node->setCPT(curr_CPT);
     		}
    	}
        Net.generateMarkovBlanket();
        myFile.close();
  	}
  	return Net;
}

int main(){
    Network net = readNet("Alarm.bif");

    std::string filename = "records.dat",line,temp;
    std::ifstream myFile(filename);
    std::vector<std::vector<int>> DataTable;
    std::vector<int> QuestionMarks;

    // Reading .dat file
    if (myFile.is_open())
    {
    	while (!myFile.eof() )
    	{
    		std::stringstream ss;
      		getline (myFile,line);
      		ss.str(line);
            std::vector<int> vals;
     		for(int i=0;i< net.getVarCount();i++){
                ss>>temp;
                if (temp.compare("\"?\"") == 0){
                    QuestionMarks.push_back(i);
                    vals.push_back(-1);
                }
                else{
                    int val = net.getNode(i)->getValToIndex(temp);
                    vals.push_back(val);
                }
            }
            DataTable.push_back(vals);
    	}
        myFile.close();
    }

    // CPT initialize
    for (int i=0;i<net.getVarCount();i++){
        int n = (net.getNode(i)->getCPT().size())/(net.getNode(i)->getnVal());
        for(int j=0;j<n;j++){
            net.getNode(i)->getCPT()[j] = 1/n;
        }
    }

    // Data -> CPT
    for(int k=0;k<DataTable.size();k++){
        for(int l=0;l<DataTable[k].size();l++){
            if (DataTable[k][l] != -1){
                //Calculate p = P(X = ? | MB)
                // curr += p;
            }
        }
    }
    // CPT -> Data


    return 0;
}
