#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <iomanip>

float SMOOTH = 0.0001;
int total_line=0;

class Node{
    int name;
    int n;
    std::vector<std::string> values;
    std::map<int,int> parents;
    std::map<std::string,int> value_to_index;
    public:
    std::vector<int> children;
    std::vector<int> parents_order;
    std::vector<int> weights;
    std::vector<float> CPT;
    Node(int i){
        this->name = i;
    }
    void addValue(std::string &val){
        if (value_to_index.find(val) != value_to_index.end()){
            return;
        }
        values.push_back(val);
        value_to_index[val] = values.size()-1;
        n++;
    }
    void addChild(int &child){
        children.push_back(child);
    }
    void addParent(int &parent){
        if (isParent(parent)){return;}
        parents[parent]= parents_order.size();
        parents_order.push_back(parent);
    }
    void setCPT(std::vector<float> &Cpt){
        CPT.clear();
        CPT = std::vector<float> (Cpt.begin(), Cpt.end());
    }
    void setParents(std::vector<int> &parent){
        for (int i=0;i<parent.size();i++){
            parents[parent[i]] = parents_order.size();
            parents_order.push_back(parent[i]);
        }
    }
    int getValToIndex(std::string &val){
        if (value_to_index.find(val) == value_to_index.end()){
            return -1;
        }
        return value_to_index[val];
    }
    bool isParent(int &parent){
        return parents.find(parent) != parents.end();
    }
    int getName(){
        return name;
    }
    int getnVal(){
        return n;
    }
};

class Network{
    std::string name;
    std::map<std::string,int> name_to_index;
    std::vector<Node*> nodes;
    public:
    std::vector<std::string> node_name;
    Network(std::string name){
        this->name = name;
    }
    Node* addNode(std::string &name){
        if (name_to_index.find(name) != name_to_index.end()){
            return nullptr;
        }
        node_name.push_back(name);
        name_to_index[name] = node_name.size()-1;
        Node * node = new Node(node_name.size()-1);
        nodes.push_back(node);
        return node;
    }
    Node* getNode(std::string &name){
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
    int getIndex(std::string &name){
        if (name_to_index.find(name) == name_to_index.end()){
            return -1;
        }
        return name_to_index.find(name)->second;
    }
    int getVarCount(){
        return node_name.size();
    }
    void setWeights(int var){
        int n=nodes[var]->parents_order.size();
        int n_ = nodes[var]->CPT.size()/nodes[var]->getnVal();
        std::vector<int> weight_(n+1);
        weight_[0] = n_;
        for(int i=0;i<n;i++){
            n_ /= nodes[nodes[var]->parents_order[i]]->getnVal();
            weight_[i+1] = n_;
        }
        nodes[var]->weights = weight_;
    }
    int calcPos(int var,std::vector<int> &values){
        // Values -> Var :: Par(Var)
        int index = 0;
        for(int i=0;i<nodes[var]->weights.size();i++){
            index += values[i]*nodes[var]->weights[i];
        }
        return index;
    }
    int name_to_index_find(std::string &name){
        return name_to_index[name];
    }
};

Network readNet(std::string FileName){
    Network Net(FileName);
    std::string line;
    std::ifstream myFile(FileName); 
    std::string temp;
    std::string name;
    std::vector<std::string> values;
    std::vector<int>parents;
    
    if (myFile.is_open())
    {
        while (!myFile.eof() )
        {
            total_line++;
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
                    for(int i =0;i<values.size();i++){
                        pos->addValue(values[i]);
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
                    
                    std::vector<float> curr_CPT;
                    std::string::size_type sz;
                    while(temp.compare(";")!=0)
                    {
                        curr_CPT.push_back(atof(temp.c_str()));
                        ss_>>temp;
                    }
                    node->setCPT(curr_CPT);
            }
        }
        for(int i=0;i<Net.getVarCount();i++){
            Net.setWeights(i);
        }
    }
    return Net;
}

int index_child_given_parents(Node* variable, int &row, int &val,std::vector<std::vector<int> >&DataTable, std::vector<int>& QuestionMarks, Network &net){
    int sz = variable->getnVal();
    int n= variable->parents_order.size();
    std::vector<int> values(n+1,0);
    values[0] = val;
    for(int j = 0; j < n; j++){
        values[j+1] = DataTable[row][variable->parents_order[j]];
    }
    return net.calcPos(variable->getName(), values);
}

float calculate_child_given_parents(Node* variable, int &row, int &val,std::vector<std::vector<int> >&DataTable, std::vector<int>& QuestionMarks, Network &net){
    int sz = variable->getnVal();
    int n= variable->parents_order.size();
    std::vector<int> values(n+1,0);
    values[0] = val;
    for(int j = 0; j < n; j++){
        values[j+1] = DataTable[row][variable->parents_order[j]];
    }
    float cpt = variable->CPT[net.calcPos(variable->getName(), values)];
    return cpt;
}

void CPT_to_data_weight(std::vector<std::vector<int> > &DataTable, std::vector<std::vector<float> > &data_weight, std::vector<int>& QuestionMarks,Network& net){
    for(int i=0;i<DataTable.size();i++){
        if(QuestionMarks[i] != -1){
            Node *variable = net.getNode(QuestionMarks[i]);
            int sz = variable->getnVal();
            float sum = 0;
            for(int j = 0; j < sz; j++){
                DataTable[i][QuestionMarks[i]] = j;
                float fact = 1;
                for(int r=0;r< variable->children.size();r++){
                    fact *= calculate_child_given_parents(net.getNode(variable->children[r]), i, DataTable[i][variable->children[r]],DataTable,QuestionMarks,net);
                }
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

void data_weight_to_CPT(std::vector<std::vector<int> > &DataTable, std::vector<std::vector<float> > &data_weight, std::vector<int>& QuestionMarks,Network& net,std::vector<std::vector<float> >&num_ds){
    for(int i=0;i<net.getVarCount();i++){
        num_ds[i] = (std::vector<float> (net.getNode(i)->CPT.size(),SMOOTH));
    }
    for(int i = 0; i < DataTable.size(); i++){
        for(int j = 0; j < DataTable[i].size(); j++){
            if(QuestionMarks[i] != j && QuestionMarks[i] != -1){
                if(!net.getNode(j)->isParent(QuestionMarks[i])){
                    int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                    num_ds[j][idx] += 1;
                }
                else{
                    for(int k = 0; k < net.getNode(QuestionMarks[i])->getnVal(); k++){
                        DataTable[i][QuestionMarks[i]] = k;
                        int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                        num_ds[j][idx] += data_weight[i][k];
                    }
                    DataTable[i][QuestionMarks[i]] = -1;
                }
            }
            else if(QuestionMarks[i] != -1){
                for(int k = 0; k < net.getNode(QuestionMarks[i])->getnVal(); k++){
                    DataTable[i][QuestionMarks[i]] = k;
                    int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                    num_ds[j][idx] += data_weight[i][k];
                }
                DataTable[i][QuestionMarks[i]] = -1;
            }
            else{
                int idx = index_child_given_parents(net.getNode(j), i, DataTable[i][j], DataTable, QuestionMarks, net);
                num_ds[j][idx] += 1;
            }
        }
    }
    for(int i = 0; i < num_ds.size(); i++){
        int v = net.getNode(i)->getnVal();
        int sz = num_ds[i].size()/v;
        for(int j = 0; j < sz; j++){
            float norm = 0;
            for(int k = 0; k < v; k++){
                norm += num_ds[i][j+sz*k];
            }
            for(int k = 0; k < v; k++){
                num_ds[i][j+sz*k] /= norm;
            }
        }
    }
    for(int i = 0; i < num_ds.size(); i++){
        for(int j =0;j<num_ds[i].size();j++){
            net.getNode(i)->CPT[j] *= 0.8;
            net.getNode(i)->CPT[j] += 0.2*num_ds[i][j];
        }
    }
    for(int i = 0; i < num_ds.size(); i++){
        int v = net.getNode(i)->getnVal();
        int sz = num_ds[i].size()/v;
        for(int j = 0; j < sz; j++){
            float norm = 0;
            for(int k = 0; k < v; k++){
                norm += net.getNode(i)->CPT[j+sz*k];
            }
            for(int k = 0; k < v; k++){
                net.getNode(i)->CPT[j+sz*k] /= norm;
            }
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
            int line_no=0;
            while (! myfile.eof() )
            {
                line_no++;
                std::stringstream ss;
                getline (myfile,line);
                ss.str(line);
                ss>>temp_input;
                if(temp_input.compare("probability")==0)
                {                        
                    ss>>temp_input;
                    ss>>temp_input;
                    int currentID = (net.name_to_index_find(temp_input));
                    outFile << line << std::endl;
                    getline (myfile,line);
                    outFile << "    table ";
                    for(int i=0; i < net.getNode(currentID)->CPT.size(); i++)
                    outFile<<std::fixed<<std::setprecision(4)<<net.getNode(currentID)->CPT[i]<<' ';
                    outFile << ";\n}";
                    if (total_line != line_no){outFile<<'\n';}
                    getline(myfile,line);
                }
                else if (line.compare("") == 0){
                    outFile<<line;
                }
                else
                    outFile <<line<<'\n';
            }
            
            myfile.close();
        }
    }

// .bif .dat .bif
int main(int argv,char* argc[]){
    time_t t = time(NULL);
    Network net = readNet(argc[1]);
    std::string filename = argc[2],line,temp;
    std::ifstream myFile(filename);
    std::vector<std::vector<int> > DataTable;
    std::vector<int> QuestionMarks;
    
    int runtime = 114;
    if (myFile.is_open())
    {
        while (!myFile.eof() )
        {
            std::stringstream ss;
            getline (myFile,line);
            ss.str(line);
            std::vector<int> vals (net.getVarCount(),0);
            int idx = -1;
            for(int i=0;i< net.getVarCount();i++){
                ss>>temp;
                if (temp.compare("\"?\"") == 0){
                    idx = i;
                    vals[i] = -1;
                }
                else{
                    vals[i] = net.getNode(i)->getValToIndex(temp);;
                }
            }
            QuestionMarks.push_back(idx);
            DataTable.push_back(vals);
        }
        myFile.close();
    }

    std::vector<std::vector<float> > data_weight (DataTable.size(),std::vector<float> (1,1));
    for (int i = 0; i < DataTable.size(); i++){
        if(QuestionMarks[i] != -1){
            std::vector<float> temp(net.getNode(QuestionMarks[i])->getnVal(), 0);
            data_weight[i] = temp;
        }
    }
    std::vector<std::vector<float> > num_ds (net.getVarCount(),std::vector<float>());
    int iter=0;
    while(time(NULL)-t < runtime){
        iter++;
        CPT_to_data_weight(DataTable,data_weight,QuestionMarks,net);
        data_weight_to_CPT(DataTable,data_weight,QuestionMarks,net,num_ds);
    }
    dataFileWriter(net,argc[1],argc[3]);
    return 0;
}
