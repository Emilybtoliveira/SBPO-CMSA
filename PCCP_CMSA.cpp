#include <bits/stdc++.h>
#include <chrono>
#include <ilcplex/ilocplex.h>


using namespace std;


//========================== data structures ===========================

class Data {
public:
    array<int, 10000> columns_set;
    array<int, 10000> columns_set_ham_distances;
    array<int, 10000> set_closest_strings;
    int columns_sets_ham_distances_size;
    int sets_closest_strings_size;
    int columns_sets_size;
    int columns_sets_ages;

    Data() : columns_sets_size(0), columns_sets_ages(0), columns_sets_ham_distances_size(0), sets_closest_strings_size(0)  {}
};

class HashMap {
public:
    void insert(int key, const Data& data) {
        table_[key] = data;
    }

    int size() {
        return table_.size();
    }

    Data* find(int key) {
        auto iter = table_.find(key);
        if (iter != table_.end()) {
            return &(iter->second);
        }
        return nullptr;
    }

    void remove(int key) {
        table_.erase(key);
        //cout << "remodivo" << key <<endl;
    }

public:
    map<int, Data> table_;
};


vector<int> bests;
HashMap columns_sets;
int starting_loop_index = 0;
int loops_with_no_improval = 0; // controle de loops sem melhora para fugir de otimos locais
int n_new_sets = 0;
int visited[100][10000];
int mapping[10000];
int columns_ILP_selection[10000];


int n, m, t, min_alpha, max_alpha; // numero de cadeias, tamanho de cada cadeia, tamanho do alfabeto
vector<char> dataset_alphabet;
vector<string> strings_dataset;
vector<vector<int>> integer_dataset;
map<char, int> alphabetMap;
vector<vector<int>> char_possibilities_per_column;

unsigned seed;


//==================== DEBUG FUNCTIONS ===============//

void print_hash(const HashMap& hash_table) {
    
    for (const auto& pair : hash_table.table_) {
        cout << "Key: " << pair.first << endl;
        const Data& data = pair.second;
        cout << "columns_sets: [ ";
        for (int i = 0; i < data.columns_sets_size; i++) {
            cout << data.columns_set[i] << " ";
        }
        cout << "]" << endl;
        cout << "columns_sets_ages: " << data.columns_sets_ages << endl;
        cout << "columns_sets_ham_distances: [ ";
        for (int i = 0; i < data.columns_sets_ham_distances_size; i++) {
            cout << data.columns_set_ham_distances[i] << " ";
        }
        cout << "]" << endl;
        cout << "sets_closest_strings: [ ";
        for (int i = 0; i < data.sets_closest_strings_size; i++) {
            cout << data.set_closest_strings[i] << " ";
        }
        cout << "]" << endl;
        cout << endl;
    }
}


//==================== INSTANCE FUNCTIONS ===============//

void generateAlphabetMapping()
{
    int i;
    for (i = 0; i < t; i++)
    {
        char alpha_char = dataset_alphabet[i];
        alphabetMap[alpha_char] = i;
    }

    min_alpha = 0;
    max_alpha = i - 1;

    /* for (const auto &item : alphabetMap)
    {
        cout << "[" << item.first << ", " << item.second << "]\n";
    } */
}

void instanceTransformFunc()
{
    vector<int> int_string;
    int element;

    for (int j = 0; j < m; j++){
        char_possibilities_per_column.push_back(int_string);
    }

    for (string cur_string : strings_dataset)
    {
        int_string.clear();

        for (/* char c : i */int j = 0; j < cur_string.size(); j++)
        {

            element = alphabetMap[cur_string[j]];
            int_string.push_back(element);

            if(char_possibilities_per_column[j].size() > 0){                
                if (find(char_possibilities_per_column[j].begin(), char_possibilities_per_column[j].end(), element) == char_possibilities_per_column[j].end()){
                    char_possibilities_per_column[j].push_back(element);  
                } 
            }
            else{
                char_possibilities_per_column[j].push_back(element);  
            }
        }

        integer_dataset.push_back(int_string);
    }
}

void readInstance()
{
    //cin >> t >> n >> m ; // tamanho do alfabeto, numero de cadeias, tamanho das cadeias
    cin  >> n >> m >> t; //  numero de cadeias, tamanho das cadeias, tamanho do alfabeto

    char cur_char;

    for (int i = 0; i < t; i++)
    {   
        cin >> cur_char;
        dataset_alphabet.push_back(cur_char);
    }

    for (int i = 0; i < n; i++)
    {
        string cur_string;
        cin >> cur_string;
        strings_dataset.push_back(cur_string);
    }

}

//==================== CMSA CORE FUNCTIONS ===============//

void solveSmallInstances()
{
    int tam = n_new_sets;

    for (int i = starting_loop_index; i < tam + starting_loop_index; i++)
    {

        Data* found_data = columns_sets.find(i);

        if (found_data->columns_sets_size == 0){
            columns_sets.remove(i);
        }
        else{

            for (int j = 0; j < found_data->columns_sets_size; j++)
            {
                int sorted_pos = rand() % (char_possibilities_per_column[found_data->columns_set[j]].size()); 
                int character = char_possibilities_per_column[found_data->columns_set[j]][sorted_pos];

                while (visited[character][found_data->columns_set[j]] == 1){
                    sorted_pos -= 1;
                    if (sorted_pos == -1) sorted_pos = char_possibilities_per_column[found_data->columns_set[j]].size() -1; 
                    character = char_possibilities_per_column[found_data->columns_set[j]][sorted_pos];
                }
                
                visited[character][found_data->columns_set[j]] = 1;
                visited[t][found_data->columns_set[j]] += 1;
                
                found_data->set_closest_strings[j] = character;
            }

            found_data->sets_closest_strings_size = found_data->columns_sets_size;
            found_data->columns_sets_ham_distances_size = n;

            for (int k = 0; k < n; k++)
            {
                int d = 0;
                for (int c = 0; c < found_data->columns_sets_size; c++)
                {
                    if(integer_dataset[k][found_data->columns_set[c]] != found_data->set_closest_strings[c]){
                        d += 1;
                    }
                }
                found_data->columns_set_ham_distances[k] = d;                
            } 
        }
    }
}

double setsSelectionSolver(int n_sets, double bsf) //CPLEX
{
    IloEnv env;
    IloModel Model(env, "Problema da Seleção de Colunas");
    double cost;

    try
    {
        // Variável de decisão
        IloIntVarArray x(env, n_sets, 0, 1);

        // Variável objetivo
        IloNumVar z(env, 0, IloInfinity, ILOINT);

        IloExprArray coluna;
        coluna = IloExprArray(env, m);

        for (int i = 0; i < m; ++i)
                coluna[i] = IloExpr(env);

        int indice = 0;
        for (const auto& pair : columns_sets.table_) {
            const Data& data = pair.second;
                for (int i = 0; i < data.columns_sets_size; i++) {
                    coluna[data.columns_set[i]] += x[indice];
                }
            mapping[indice] = pair.first;
            indice += 1;
        }

        for (int i = 0; i < m; ++i) Model.add(coluna[i] == 1);

        for (int j = 0; j < n; j++)
        {
            IloExpr exp2(env);

            for (int s = 0; s < indice; s++)
            {
                int key = mapping[s];
                
                Data* found_data = columns_sets.find(key);

                exp2 += found_data->columns_set_ham_distances[j] * x[s];
            }

            Model.add(exp2 <= z);
        }

        // restrição p podar mais rapido
        //Model.add(z < bsf);

        // Função objetivo.
        Model.add(IloMinimize(env, z));

        // Solving
        IloCplex cplex(Model);
        cplex.setOut(env.getNullStream());
        cplex.setParam(IloCplex::Param::TimeLimit, 0.5); // limite de tempo pra resolver
        //cplex.setParam(IloCplex::EpGap,  0.05); //set the minimum required gap
        //cout << cplex.getModel() << endl;

        IloNumVarArray startVar(env);
        IloNumArray startVal(env);

        if (cplex.solve()){
            cost = cplex.getObjValue();
            
            // cout << cplex.getValue(z) << endl ;
            //cout << cplex.getObjValue() << endl << endl;
            //cout << cplex.getCplexStatus() << endl;

            // Obtendo a solução
            IloNumArray sol(env, n_sets);
            cplex.getValues(sol, x);
            
            bests.clear();
            for (int i = 0; i < n_sets; i++)
            {
                if (sol[i] > 0.5){
                    bests.push_back(mapping[i]);
                     columns_ILP_selection[i] = 1;

                }
                     

                else{
                    columns_ILP_selection[i] = 0;
                } 
                
            }

            cplex.end();
            Model.end();
            env.end();
       
            return cost;
        }
    }
    catch (const IloException &e)
    {
        cerr << "Exception caught: " << e << endl;
    }
    catch (...)
    {
        cerr << "Unknown exception caught!" << endl;
    }

    return bsf; // se nao encontrou, retorna a melhor que já tem
}


void adapt(int max_age)
{
    int tam = columns_sets.size();
    for (int i = 0; i < tam; i++)
    {
        int key = mapping[i];
        Data* found = columns_sets.find(key);
        if(found == nullptr) cout << "erro 512: ponteiro nulo" << endl;
        if(found->columns_sets_size == 0) cout << "erro 554: elemento vazio" << endl;

        if (columns_ILP_selection[i] == 0)
        {
            found->columns_sets_ages += 1;
            if (found->columns_sets_ages >= max_age)
            {
                for (int j = 0; j < found->sets_closest_strings_size ; j++)
                {
                    int character = found->set_closest_strings[j];
                    int coluna = found->columns_set[j];
                    visited[character][coluna] = 0;
                    visited[t][coluna] -= 1;
                }

                columns_sets.remove(key);                    
            }
        }
        else
        {
            found->columns_sets_ages = 0;
        }
    }

}


void columnsSetsGenerator(int loop, int max_size, int min_size)
{
    int set, i, j, l;

    float columns_partition_size = 1;

    vector<int> shuffled_sets;
    vector<int> empty_vec;
    
    columns_partition_size = (rand() % max_size) + min_size; 
    int new_sets = ceil((float) m / columns_partition_size);
    int index = starting_loop_index;

    
    //================= loop da criação dos conjuntos ==========================
    for (l = 0; l < loop; l++)
    {
        
        
        // popula o shuffled_sets com os indices dos conjuntos em quantidade igual a columns_partition_size
        for (int i = index ; i < new_sets + index; i++)
        {            
            for (j = 0; j < columns_partition_size; j++)
            {
                shuffled_sets.push_back(i);
            }
        }

        seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(shuffled_sets.begin(), shuffled_sets.end(), default_random_engine(seed));


        for (int i = index; i < new_sets + index; i++) {

            Data new_data;
            Data* found_data = columns_sets.find(i);

            if(found_data != nullptr){
                cout << "erro 116:" << i << endl;
            }
            else{
                columns_sets.insert(i, new_data);
            }
            
        }

        j = 0;
        for (i = 0; i < m; i++)
        {
            // column = rand() % n_new_sets;
            if (!(visited[t][i] == char_possibilities_per_column[i].size())){
                set = shuffled_sets[j];
                Data* found_data = columns_sets.find(set);
                found_data->columns_set[found_data->columns_sets_size] = i;
                found_data->columns_sets_size +=1;
            }
            
            j++;
        }

        index = index + new_sets;
        shuffled_sets.clear();
    }
    n_new_sets = new_sets*loop;    
} 


void mainLoop(int instance_opt, float time_limit)
{
    double opt, bsf = m;
    int loops = 0;

    auto loop_start = chrono::high_resolution_clock::now();
    auto loop_end = loop_start;
    auto loop_cur = chrono::duration_cast<chrono::milliseconds>(loop_end - loop_start);
    
    while (loops < 20 and loops_with_no_improval < 5)
    {
              
        //================= CMSA Procedures ==========================
        columnsSetsGenerator(1,1,1);   
          
        solveSmallInstances();    
        opt = setsSelectionSolver(columns_sets.size(), bsf); // seleciona os conjuntos

        if (opt < bsf) // update da melhor solução
        {         
            loops_with_no_improval = 0;
            bsf = opt;
        }
        else
        {
            loops_with_no_improval += 1;
        }     
        
        adapt(1); // envelhecimento

        loops += 1;
        loop_end = chrono::high_resolution_clock::now();
        loop_cur = chrono::duration_cast<chrono::milliseconds>(loop_end - loop_start);
        starting_loop_index = starting_loop_index + n_new_sets;
    }

    cout << "loops: " << loops << endl;
    cout <<"time: " << loop_cur.count() << endl;
    cout <<"opt: " << bsf << endl;
    
}


int main(int argc, char *argv[])
{
    int opt;
    float time_limit;

    for(int i = 0; i < 100;i++){
        for(int j = 0; j < 10000; j++){
            visited[i][j] = 0;
        }
    }

    readInstance();
    //printInstance();
    generateAlphabetMapping();
    instanceTransformFunc();

    //lendo argumentos da linha de comando
    /* for (int i = 0; i < argc; i++) {
        if(!strcmp(argv[i], "-opt")) {          
            sscanf(argv[i+1], "%d", &opt);
            i++;
        }
        if(!strcmp(argv[i], "-t")) {          
            sscanf(argv[i+1], "%f", &time_limit);
            i++;
        }
    }
    cout << "opt received: " << opt << endl; */
    
    
    mainLoop(opt, time_limit);

    return 0;
}