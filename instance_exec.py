import subprocess
import os

def writeCSV(line, arquivo):
    arquivo = open("./SBPO_results/"+arquivo, "a")
    arquivo.write(line)
    arquivo.close()

def writeLog(line, arquivo):
    arquivo = open(arquivo, "a")
    arquivo.write(line)
    arquivo.close()

def skipInstance(instance, caminho):
   with open(caminho, 'r') as arquivo:
    conteudo = arquivo.read()
    
    arquivo.close()
    return True if instance in conteudo else False


# Define o caminho para o diretório com o código C++ e o Makefile
path_CMSA = './CMSA2'
path_CPLEX = '../PCCP-CPLEX'
path_instance = './SBPO_instances/SBPO'
path_log = './SBPO_logs/'

# Define o comando para executar o makefile
make_CMSA_command = ['make', '-C', path_CMSA]
make_CPLEX_command = ['make', '-C', path_CPLEX]

# Executa o makefile CMSA
make_result = subprocess.run(make_CMSA_command, capture_output=True, text=True)
print(make_result.stdout)


# Executa o makefile CPLEX
make_result = subprocess.run(make_CPLEX_command, capture_output=True, text=True)
print(make_result.stdout)

""" arquivo = open("./SBPO_results/results_CPLEX.csv", "w")
arquivo.write("instance&best&mean&time_avg\n")"""

""" arquivo = open("./SBPO_results/results_CMSA_2.csv", "w")
arquivo.write("instance&best&mean&time_avg\n")  """

for instance_dir in sorted(os.listdir(path_instance)):
    for sub_instance_dir in sorted(os.listdir(path_instance + "/" +instance_dir)):
        avg_result_CMSA = 0
        avg_time_CMSA = 0 
        best_CMSA = 0        
        
        if(not skipInstance(sub_instance_dir,  "./SBPO_results/results_CMSA_2.csv")):
            for instance_file in sorted(os.listdir(path_instance+ "/" +instance_dir+"/"+sub_instance_dir)):           
                log_file = "./"+path_log+"/CMSA/"+instance_dir+"/"+sub_instance_dir+".txt"
                print(f"-------------{instance_file}-------------")
                #writeLog(f"\n-------------{instance_file}-------------\n", log_file)

                program_name = os.path.join(path_CMSA, 'PCCP_CMSA_2.run')
                file_name = os.path.join(path_instance, instance_dir, sub_instance_dir, instance_file)

                program_result = subprocess.run([program_name], stdin=open(file_name), capture_output=True, text=True)

                #transforma em dicionario
                output = program_result.stdout
                output = output.split("\n")
                output.remove('')

                dictionary = dict(item.split(": ") for item in output) 
                #print(dictionary) 
                #writeLog(str(dictionary)+"\n", log_file)

                avg_result_CMSA += int(dictionary['opt'])
                avg_time_CMSA += int(dictionary['time'])

                if(int(dictionary['opt']) < best_CMSA or best_CMSA == 0): 
                    best_CMSA = int(dictionary['opt'])
                    best_time_CMSA = int(dictionary['time'])
        
            writeCSV(sub_instance_dir+'&'+str(best_CMSA)+'&'+str(avg_result_CMSA/20)+'&'+str(avg_time_CMSA/20)+"\\\ \n", "results_CMSA_2.csv")


""" for instance_dir in sorted(os.listdir(path_instance)):
    for sub_instance_dir in sorted(os.listdir(path_instance + "/" +instance_dir)):
        avg_time_CPLEX = 0
        avg_result_CPLEX = 0
        best_CPLEX = 0

        if(not skipInstance(sub_instance_dir, "./SBPO_results/results_CPLEX.csv")):
            log_file = "./"+path_log+"/CPLEX/"+instance_dir+"/"+sub_instance_dir+".txt"
            
            for instance_file in sorted(os.listdir(path_instance+ "/" +instance_dir+"/"+sub_instance_dir)):                    
                print(f"-------------{instance_file}-------------")
                writeLog(f"\n-------------{instance_file}-------------\n", log_file)                
                
                program_name = os.path.join(path_CPLEX, 'PCCP_cplex.run')
                file_name = os.path.join(path_instance, instance_dir, sub_instance_dir, instance_file)
                program_result = subprocess.run([program_name], stdin=open(file_name), capture_output=True, text=True)

                #transforma em dicionario
                output = program_result.stdout
                output = output.split("\n")
                output.remove('')

                dictionary = dict(item.split(": ") for item in output) 
                print(dictionary) 
                writeLog(str(dictionary)+"\n", log_file)
                
                avg_result_CPLEX += int(dictionary['opt'])
                avg_time_CPLEX += int(dictionary['time'])

                if(int(dictionary['opt']) < best_CPLEX or best_CPLEX == 0): 
                    best_CPLEX = int(dictionary['opt'])
                    best_time_CPLEX = int(dictionary['time']) 
                        
            writeCSV(sub_instance_dir+'&'+str(best_CPLEX)+'&'+str(avg_result_CPLEX/20)+'&'+str(avg_time_CPLEX/20)+"\\\ \n", "results_CPLEX.csv")
"""         
            
