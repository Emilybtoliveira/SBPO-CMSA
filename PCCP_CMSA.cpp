#include <bits/stdc++.h>
#include <chrono>
#include <ilcplex/ilocplex.h>

using namespace std;
#define MAX_SETS 100000 //NOTE: Limita o tamanho das strings em MAX_SETS/(max_age+1) = 50000
#define MAX_SET_SIZE 1000
#define MAX_ALPHABET 26
#define INF -574873548

//========================== data structures ===========================

class Set {
public:
    int size;
    int age;
    array<int, MAX_SET_SIZE> columns;
    array<int, MAX_SETS> hamming_dist; /* NOTE: O tamanho de hamming distance é sempre maior, 
                                          porque sempre é igual a quantidade de strings de entrada (n) */
    array<int, MAX_SET_SIZE> closest_string;

    Set() : size(0), age(0) {}
};

class HashMap {
public:
    void insert(int key, const Set& set) {
        table_[key] = set;
    }

    int size() {
        return table_.size();
    }

    Set* find(int key) {
        auto iter = table_.find(key);
        if (iter != table_.end()) {
            return &(iter->second);
        }
        return nullptr;
    }

    void remove(int key) {
        table_.erase(key);
    }

public:
    map<int, Set> table_;
};

int n, m, t, min_alpha, max_alpha; // numero de cadeias, tamanho de cada cadeia, tamanho do alfabeto
vector<char> dataset_alphabet;
vector<string> strings_dataset;
vector<vector<int>> integer_dataset;
map<char, int> alphabetMap;
vector<vector<int>> char_possibilities_per_column;

HashMap columns_sets;
int starting_loop_index = 0;
int loops_with_no_improval = 0; // controle de loops sem melhora para fugir de otimos locais
int loops = 0;
int n_new_sets = 0;
int visited[MAX_ALPHABET][MAX_SETS]; 
int mapping[MAX_SETS]; 
int columns_ILP_selection[MAX_SETS];

unsigned seed = 1;

//==================== DEBUG FUNCTIONS ===============//

void print_hash(const HashMap& hash_table) {
    
    for (const auto& pair : hash_table.table_) {
        cout << "Key: " << pair.first << endl;
        const Set& set = pair.second;
        cout << "columns_sets: [ ";
        for (int i = 0; i < set.size; i++) {
            cout << set.columns[i] << " ";
        }
        cout << "]" << endl;
        cout << "age: " << set.age << endl;
        cout << "columns_sets_ham_distances: [ ";
        for (int i = 0; i < n; i++) {
            cout << set.hamming_dist[i] << " ";
        }
        cout << "]" << endl;
        cout << "sets_closest_strings: [ ";
        for (int i = 0; i < set.size; i++) {
            cout << set.closest_string[i] << " ";
        }
        cout << "]" << endl;
        cout << endl;
    }
}

int* generateClosestString(){
    int* closest_string = new int[m];
    int n_sets = columns_sets.size();
    int key;
    Set* found;

    for (int i = 0; i < n_sets; i++)
    {
        if (columns_ILP_selection[i] == 1)
        {            
            key = mapping[i];
            found = columns_sets.find(key);

            for (int j = 0; j < found->size; j++){
                closest_string[found->columns[j]] = found->closest_string[j];
            }
        }
    }

/*     for(int i = 0; i < m; i++){
        cout << closest_string[i] << " ";
    }
    cout << endl; */

    return closest_string;
}

void optionsAndSelected(){
    int* closest_string = generateClosestString();
    cout << "Loop " << loops << endl;

    for(int j = 0; j < m; j ++){
        printf("Caracteres ativos da coluna %d:", j);

        for(int i = 0; i < t; i++){
            if(visited[i][j] == 1){
                cout << i << " ";
            }
        }
        cout << endl;

        printf("\tCaractere escolhido pelo CPLEX: %d\n", closest_string[j]);
    }
    cout << endl;
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

//inicializa vetor de visitados
void initializeDataStructures(){
    for(int i = 0; i < MAX_ALPHABET;i++){
        for(int j = 0; j < MAX_SETS; j++){
            visited[i][j] = 0;
        }
    }

}

//==================== CMSA CORE FUNCTIONS ===============//

void solveSmallInstances()
{
    int tam = n_new_sets;

    for (int i = starting_loop_index; i < tam + starting_loop_index; i++)
    {

        Set* found_data = columns_sets.find(i);

        if (found_data->size == 0){
            columns_sets.remove(i);
        }
        else{

            for (int j = 0; j < found_data->size; j++)
            {
                int column = found_data->columns[j];
                int sorted_pos = rand() % (char_possibilities_per_column[column].size());  //escolhe aleatoriamente um caractere para a posição
                int character = char_possibilities_per_column[column][sorted_pos];

                
                while (visited[character][column] == 1){ 
                    /* aqui o loop varre na direção antihorario as linhas de caracteres até encontrar uma posição em que o caractere n esteja sendo utilizado*/
                    
                    sorted_pos -= 1;
                    if (sorted_pos == -1) sorted_pos = char_possibilities_per_column[column].size() -1; 
                    character = char_possibilities_per_column[column][sorted_pos];
                }


                visited[character][column] = 1;
                visited[t][column] += 1;
                
                found_data->closest_string[j] = character;
            }

            for (int k = 0; k < n; k++)
            {
                int d = 0;
                for (int c = 0; c < found_data->size; c++)
                {
                    if(integer_dataset[k][found_data->columns[c]] != found_data->closest_string[c]){
                        d += 1;
                    }
                }

                found_data->hamming_dist[k] = d;                
            } 
        }
    }
}

//CPLEX
double solve(int n_sets, double bsf, float time_limit) 
{
    IloEnv env;
    IloModel Model(env, "Problema da Seleção de Conjuntos");
    double cost;


    try
    {
        // Variável de decisão
        IloIntVarArray x(env, n_sets, 0, 1);

        // Variável objetivo
        IloNumVar z(env, 0, IloInfinity, ILOINT);

        IloExprArray column;
        column = IloExprArray(env, m);

        for (int i = 0; i < m; ++i) column[i] = IloExpr(env);

        int indice = 0;
        for (const auto& pair : columns_sets.table_) {
            const Set& set = pair.second;

            for (int i = 0; i < set.size; i++) {
                column[set.columns[i]] += x[indice];
            }

            mapping[indice] = pair.first;
            indice += 1;
        }

        for (int i = 0; i < m; ++i) Model.add(column[i] == 1);

        for (int j = 0; j < n; j++)
        {
            IloExpr exp2(env);

            for (int s = 0; s < indice; s++)
            {
                int key = mapping[s];
                
                Set* found_data = columns_sets.find(key);

                exp2 += found_data->hamming_dist[j] * x[s];
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
        //cplex.setParam(IloCplex::Param::TimeLimit, 0.5); // limite de tempo pra resolver
        cplex.setParam(IloCplex::Param::TimeLimit, time_limit); // limite de tempo pra resolver
        //cplex.setParam(IloCplex::EpGap,  0.05); //set the minimum required gap
        //cout << cplex.getModel() << endl;


        if (cplex.solve()){
            cost = cplex.getObjValue();
            
            // cout << cplex.getValue(z) << endl ;
            //cout << cplex.getObjValue() << endl << endl;
            //cout << cplex.getCplexStatus() << endl;

            // Obtendo a solução
            IloNumArray sol(env, n_sets);
            cplex.getValues(sol, x);
            
            for (int i = 0; i < n_sets; i++)
            {
                if (sol[i] > 0.5){
                    columns_ILP_selection[i] = 1;
                } else{
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
        Set* found = columns_sets.find(key);
        if(found == nullptr) cout << "erro 512: ponteiro nulo" << endl;
        if(found->size == 0) cout << "erro 554: elemento vazio" << endl;

        if (columns_ILP_selection[i] == 0)
        {
            found->age += 1;
            
            if (found->age >= max_age)
            {
                for (int j = 0; j < found->size ; j++)
                {
                    int character = found->closest_string[j];
                    int column = found->columns[j];
                    visited[character][column] = 0;
                    visited[t][column] -= 1;
                }

                columns_sets.remove(key);                    
            }
        }
        else
        {
            found->age = 0;

            for (int j = 0; j < found->size ; j++)
            {
                int character = found->closest_string[j];
                int column = found->columns[j];

            }
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

        seed = chrono::system_clock::now().time_since_epoch().count(); //a seed variável mantém o processo estocástico ao longo loops
        shuffle(shuffled_sets.begin(), shuffled_sets.end(), default_random_engine(seed));

        for (int i = index; i < new_sets + index; i++) {
            Set new_data;
            Set* found_data = columns_sets.find(i);

            if(found_data == nullptr) columns_sets.insert(i, new_data);                     
        }

        j = 0;
        for (i = 0; i < m; i++)
        {
            /* A ultima linha (t) da matriz visited de cada coluna(conjunto) diz a quantidade de caracteres ja utilizada para a coluna. Nesse if, 
            verifica-se se essa quantidade  é igual ao limite de possibilidades de caracteres. Se for, não será adicionado mais nenhuma. */
            if (!(visited[t][i] == char_possibilities_per_column[i].size())){
                set = shuffled_sets[i];
                Set* found_data = columns_sets.find(set);
                found_data->columns[found_data->size] = i;
                found_data->size +=1;
            }   
             
            j++;        
        }

        index = index + new_sets;
        shuffled_sets.clear();
    }
    n_new_sets = new_sets*loop;    
} 


void CMSA(float time_limit, int max_age, int max_loops)
{
    double opt, bsf = m;

    auto loop_start = chrono::high_resolution_clock::now();
    auto loop_end = loop_start;
    auto loop_cur = chrono::duration_cast<chrono::milliseconds>(loop_end - loop_start);    
    
    initializeDataStructures();

    //================= CMSA Loop ==========================

    while (loops_with_no_improval < 5)
    {              
        //CONSTRUCT + MERGE
        columnsSetsGenerator(1,1,1);     
        solveSmallInstances();    

        //SOLVE
        opt = solve(columns_sets.size(), bsf, time_limit); // seleciona os conjuntos

        if (opt < bsf) // update da melhor solução
        {         
            loops_with_no_improval = 0;
            bsf = opt;
        }
        else
        {
            loops_with_no_improval += 1;
        }     
        
        /* DEBUG */
        //generateClosestString();
        //optionsAndSelected();

        //ADAPT
        adapt(max_age); // envelhecimento


        loops += 1;
        loop_end = chrono::high_resolution_clock::now();
        loop_cur = chrono::duration_cast<chrono::milliseconds>(loop_end - loop_start);
        starting_loop_index = starting_loop_index + n_new_sets;
    }

    cout << "loops: " << loops << endl;
    cout <<"time: " << loop_cur.count() << endl;
    cout <<"opt: " << bsf << endl;
    //printf("%.f [%ld]\n", bsf, loop_cur.count());
    /* cout << bsf; */
}

int main(int argc, char *argv[])
{
    int max_age = 0, max_loops = 0;
    float time_limit = 0;    

    readInstance();
    //printInstance();
    generateAlphabetMapping();
    instanceTransformFunc();

    //lendo argumentos da linha de comando
    for (int i = 0; i < argc; i++) {
        
        if(!strcmp(argv[i], "-t")) {          
            sscanf(argv[i+1], "%f", &time_limit);
            i++;
        }
        if(!strcmp(argv[i], "-a")) {          
            sscanf(argv[i+1], "%d", &max_age);
            i++;
        }
        if(!strcmp(argv[i], "-l")) {          
            sscanf(argv[i+1], "%d", &max_loops);
            i++;
        }
    }

    if(!time_limit){ cout << "Missing -t" << endl; return 0 ;}

    //cout << time_limit << " " << max_age << " " << max_loops << endl;

    CMSA(time_limit, max_age, max_loops);

    return 0;
}