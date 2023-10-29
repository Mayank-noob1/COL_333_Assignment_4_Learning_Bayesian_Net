#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <map>
#include <set>
#include <ctime>

const double SMOOTH = 0.01;

class Node{
    // Name
    int name;                       // f : () -> name
    int n;
    // n - arity
    std::vector<std::string> values;        // f : i -> name

    // Markov blanket - Parent(Child)
    std::map<int,int> parents;       // f : i -> parent
    std::set<int> markovBlanket;
    // CPT
    public:
    std::map<std::string,int> value_to_index;    // f : name -> i
    std::set<int> children;      // f : i -> child
    std::vector<int> parents_order;
    std::vector<int> weights;
    std::vector<double> CPT;
    Node(int i){
        this->name = i;
    }
    Node (int i,std::vector<std::string> values){
        this->name = i;
        this->values = values;
        this->n = values.size();value_to_index.clear();
    }
    void addValue(std::string val){
        if (value_to_index.find(val) != value_to_index.end()){
            std::cout<<"Yooooooooooooooooooo\n";
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
        // if (value_to_index.find(val) == value_to_index.end()){
        //     return -1;
        // }
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
};

class Network{
   // Name
    std::string name;                       // f : () -> name

    // n - arity
    std::vector<std::string> node_name;        // f : i -> node_name

    public:
    std::map<std::string,int> name_to_index;    // f : node_name -> i
    std::vector<Node*> nodes;                    // f: i -> node *
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
    void setWeights(int var){
        int n=nodes[var]->parents_order.size();
        int n_ = nodes[var]->CPT.size()/nodes[var]->getnVal();
        nodes[var]->weights.push_back(n_);
        for(int i=0;i<n;i++){
            n_ /= nodes[nodes[var]->parents_order[i]]->getnVal();
            nodes[var]->weights.push_back(n_);
        }
    }
    int calcPos(int var,std::vector<int> &values){
        // Values -> Var :: Par(Var)
        int index = 0;
        for(int i=0;i<nodes[var]->weights.size();i++){
            index += values[i]*nodes[var]->weights[i];
        }
        return index;
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
                    Node* pos=Net.addNode(name);
                    for(auto &value:values){
                        pos->addValue(value);
                    }
            }
            else if(temp.compare("probability")==0)
            {
                    ss>>temp;
                    ss>>temp;

                    Node* node=Net.getNode(temp);
                    int index=Net.getIndex(temp);
                    ss>>temp;
                    values.clear();
                    parents.clear();
                    while(temp.compare(")")!=0)
                    {
                        Node* node_=Net.getNode(temp);
                        node_->addChild(index);
                        int index_ = Net.getIndex(temp);
                        if (index_ == -1){
                            Net.addNode(temp);
                            std::cout<<"Fucked---------------------------------------\n";
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
        for(int i=0;i<Net.getVarCount();i++){
            Net.setWeights(i);
        }
        // myFile.close();
    }
    return Net;
}

int index_child_given_parents(Node* variable, int row, int val,std::vector<std::vector<int> >&DataTable, std::vector<int>& QuestionMarks, Network &net){
    int sz = variable->getnVal();
    std::vector<int> values;
    values.push_back(val);
    for(int j = 0; j < variable->parents_order.size(); j++){
        values.push_back(DataTable[row][variable->parents_order[j]]);
    }
    return net.calcPos(variable->getName(), values);
}

double calculate_child_given_parents(Node* variable, int row, int val,std::vector<std::vector<int> >&DataTable, std::vector<int>& QuestionMarks, Network &net){
    int sz = variable->getnVal();
    std::vector<int> values;
    values.push_back(val);
    for(int j = 0; j < variable->parents_order.size(); j++){
        values.push_back(DataTable[row][variable->parents_order[j]]);
    }
    double cpt = variable->CPT[net.calcPos(variable->getName(), values)];
    return cpt;
}

void CPT_to_data_weight(std::vector<std::vector<int> > &DataTable, std::vector<std::vector<double> > &data_weight, std::vector<int>& QuestionMarks,Network& net){
    for(int i=0;i<DataTable.size();i++){
        if(QuestionMarks[i] != -1){
            Node *variable = net.getNode(QuestionMarks[i]);
            int sz = variable->getnVal();
            double sum = 0;
            for(int j = 0; j < sz; j++){
                DataTable[i][QuestionMarks[i]] = j;
                double fact = 1;
                for(auto &child : variable->children){
                    // product child | parent
                    fact *= calculate_child_given_parents(net.getNode(child), i, DataTable[i][child],DataTable,QuestionMarks,net);
                }
                // Node | parent
                data_weight[i][j] = fact*(calculate_child_given_parents(variable, i, j,DataTable,QuestionMarks,net));
                sum += data_weight[i][j];
            }
            for(int j=0;j<sz;j++){
                data_weight[i][j] /= sum;
            }
            DataTable[i][QuestionMarks[i]] =-1;
        }
    }
    return;
}

// Updating CPT values using the data_file and data_weights (with smoothing)
void data_weight_to_CPT(std::vector<std::vector<int> > &DataTable, std::vector<std::vector<double> > &data_weight, std::vector<int>& QuestionMarks,Network& net){
    std::vector<std::vector<double> > num_ds;  // Data structure containing numerator values of CPT of variables.
    for(int i=0;i<net.getVarCount();i++){
        num_ds.push_back(std::vector<double> (net.getNode(i)->CPT.size(),SMOOTH));
    }
    for(int i = 0; i < DataTable.size(); i++){                           // Iterating over the data table
        for(int j = 0; j < DataTable[i].size(); j++){                    // Iterating over row of the data table
            if(QuestionMarks[i] != j && QuestionMarks[i] != -1){         // Checking if the node is not the missing node
                if(!net.nodes[j]->isParent(QuestionMarks[i])){      // Checking if the missing node is not one of the parent of the node (This is wrong syntax but hopefully you get the meaning)
                    int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                    // std::cout<<idx<<' '<<num_ds[j].size()<<'\n';
                    num_ds[j][idx] += 1;
                }
                else{
                    for(int k = 0; k < net.getNode(QuestionMarks[i])->getnVal(); k++){
                        DataTable[i][QuestionMarks[i]] = k;
                        int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                        // std::cout<<idx<<' '<<num_ds[j].size()<<'\n';
                        num_ds[j][idx] += data_weight[i][k];
                    }
                    DataTable[i][QuestionMarks[i]] = -1;
                }
            }
            else if(QuestionMarks[i] != -1){
                for(int k = 0; k < net.getNode(QuestionMarks[i])->getnVal(); k++){
                    DataTable[i][QuestionMarks[i]] = k;
                    int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                    // std::cout<<idx<<' '<<num_ds[j].size()<<'\n';
                    num_ds[j][idx] += data_weight[i][k];
                }
                DataTable[i][QuestionMarks[i]] = -1;
            }
            else{
                std::cout << "row did not contain Question Mark" << std::endl;
            }
        }
    }

    // Now normalize the values in num_ds
    for(int i = 0; i < num_ds.size(); i++){
        // for(int j =0;j<num_ds[i].size();j++){
        //     std::cout<<num_ds[i][j]<<' ';
        // }
        // std::cout<<'\n';
        int v = net.getNode(i)->getnVal();
        int sz = num_ds[i].size()/v;
        for(int j = 0; j < sz; j++){
            double norm = 0;
            for(int k = 0; k < v; k++){
                norm += num_ds[i][j+sz*k];
            }
            for(int k = 0; k < v; k++){
                num_ds[i][j+sz*k] /= norm;
            }
        }
    }
    for(int i = 0; i < num_ds.size(); i++){
        net.nodes[i]->CPT = num_ds[i];
    }
}

void writeFile(Network &net,std::string outFile){
    {
        std::string line;
        std::ofstream myfile(outFile); 
        for(int i=0;i<net.getVarCount();i++){
            for(int j=0;j<net.nodes[i]->weights.size();j++){
                myfile<<net.nodes[i]->weights[j]<<' ';
            }
            myfile<<'\n';
        }
    }
}


void dataFileWriter(Network &net,std::string filename,std::string outfilename) 
    {
        std::string line;
        std::ifstream myfile(filename); 
        std::ofstream outFile (outfilename);
        std::string temp_input;
        std::string name;
        if (! myfile.is_open())
            return;
        else
        {
            while (! myfile.eof() )
            {
                std::stringstream ss;
                getline (myfile,line);
                ss.str(line);
                ss>>temp_input;
                if(line.compare("")==0)
                    outFile << line << std::endl;
                else if(temp_input.compare("probability")==0)
                {                        
                    ss>>temp_input;
                    ss>>temp_input;
                    int currentID = (net.name_to_index.find(temp_input)->second);
                    // outFile<<currentID<<std::endl;
                    outFile << line << std::endl;
                    getline (myfile,line);
                    outFile << "    table ";
                    for(int i=0; i < net.getNode(currentID)->CPT.size(); i++)
                    outFile<<std::fixed<<std::setprecision(4)<<net.getNode(currentID)->CPT[i]<<' ';
                        // printf("%.4f ",net.getNode(currentID)->CPT[i]);
                    outFile << ";" << std::endl;
                    // std::cout<<net.getNode(currentID)->CPT.size()<<'\n';
                }
                else if (line.compare("\n") == 0)
                {
                    outFile<< line;

                }
                else 
                    outFile << line<<std::endl;
            }
            
            myfile.close();
        }
    }
int main(){
    time_t t = time(NULL);
    Network net = readNet("Alarm.bif");
    std::ofstream myfile("outfile.bif"); 
    std::string filename = "records.dat",line,temp;
    std::ifstream myFile(filename);
    std::vector<std::vector<int> > DataTable;
    std::vector<int> QuestionMarks;
    std::vector<std::vector<double> > data_weight;
    // Reading .dat file
    if (myFile.is_open())
    {
        while (!myFile.eof() )
        {
            std::stringstream ss;
            getline (myFile,line);
            ss.str(line);
            std::vector<int> vals;
            int idx = -1;
            for(int i=0;i< net.getVarCount();i++){
                ss>>temp;
                if (temp.compare("\"?\"") == 0){
                    idx = i;
                    vals.push_back(-1);
                }
                else{
                    int val = net.getNode(i)->getValToIndex(temp);
                    vals.push_back(val);
                }
            }
            QuestionMarks.push_back(idx);
            DataTable.push_back(vals);
            vals.clear();
        }
        myFile.close();
    }

    // CPT initialize
    for (int i=0;i<net.getVarCount();i++){
        double n_ = 1.0/(double)net.getNode(i)->getnVal();
        for(int j=0;j<net.getNode(i)->CPT.size();j++){
            net.getNode(i)->CPT[j] = n_;
        }
    }

    for(int i=0;i<net.getVarCount();i++){
        for (int i=0;i<net.getVarCount();i++){
        int n = net.getNode(i)->getnVal();
        for(int j=0;j<net.getNode(i)->CPT.size();j++){
            myfile<<net.getNode(i)->CPT[j] << ' ';
        }
        myfile<<'\n';
        }
    }

    // Data weight initialize
    for (int i = 0; i < DataTable.size(); i++){
        if(QuestionMarks[i] != -1){
            std::vector<double> temp(net.getNode(QuestionMarks[i])->getnVal(), 0);
            data_weight.push_back(temp);
        }
        else{
            std::vector<double> temp(1,1);
            data_weight.push_back(temp);
        }
        
    }
    
    int iter=0;
    while(time(NULL)-t < 10){
        iter++;
    CPT_to_data_weight(DataTable,data_weight,QuestionMarks,net);
    data_weight_to_CPT(DataTable,data_weight,QuestionMarks,net);
    }

    dataFileWriter(net,"Alarm.bif","solved_alarm.bif");

    return 0;
}
