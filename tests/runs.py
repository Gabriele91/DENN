import os
import subprocess
import numpy
import pandas
import zipfile
import argparse
import copy
#consts
PATH_RESDIR="results"
TEMPLATE_OUTPUT="{}_{}.json"
CALL_OUTPUT_STD = "res.stdout"
CALL_OUTPUT_STDERR = "res.stderr"
ROOT = "tests"
#from args
PATH_DENN="Release/DENN-float"
PATH_TMPDIR=os.path.join(ROOT,"_tmp_runs_")
PATH_INSTANCES=os.path.join(ROOT,"instances.txt")
NRUNS=10

def execute_denn(denn, tmpdir, template, name, args, idrun, get_only_output=False):
    name_output = TEMPLATE_OUTPUT.format(name,idrun)
    full_output = os.path.join(tmpdir,PATH_RESDIR,name_output)
    path_stdout = os.path.join(tmpdir,CALL_OUTPUT_STD)
    path_stderr = os.path.join(tmpdir,CALL_OUTPUT_STDERR)
    if not get_only_output:
        with open(path_stdout,"a+") as ofile:
            with open(path_stderr,"a+") as errfile:
                subprocess.call([denn, template, "full_output={}".format(full_output), args], 
                                stdout=ofile, 
                                stderr=errfile)
    return full_output

def make_output_dir(tmpdir): 
    os.makedirs(tmpdir, exist_ok=True)
    os.makedirs(os.path.join(tmpdir,PATH_RESDIR), exist_ok=True)

def exe_instance(denn, tmpdir, runs, template, name, args, get_only_output=False):
    outputs = []
    for idrun in range(runs):
        outputs.append(execute_denn(denn, tmpdir, template, name, args, idrun, get_only_output=get_only_output))
    return outputs

def get_result_from_json(output):
    import json
    with open(output) as fjson:
        outjson = json.load(fjson)
    return float(outjson["accuracy"])

def get_time_from_json(output):
    import json
    with open(output) as fjson:
        outjson = json.load(fjson)
    return float(outjson["time"])

def outputs_to_results(outputs):
    results = []
    for output in outputs:
        results.append(get_result_from_json(output))
    return results

def outputs_to_times(outputs):
    times = []
    for output in outputs:
        times.append(get_time_from_json(output))
    return times

def save_csv_of_dic(dic, path):
    dataframe = pandas.DataFrame(dic)
    csvstring = dataframe.to_csv(index=False)
    with open(path,"w") as fcsv:
        fcsv.write(csvstring)

def mean_var_std(outputs):
    npoutputs = numpy.array(outputs)
    return [npoutputs.mean(),npoutputs.var(),npoutputs.std()]

def append_all_outputs(dic):
    outputs_all = []
    for key, outputs in dic.items():
        outputs_all.extend(outputs)
    return outputs_all

def save_all_in_zipfile(tmpdir, files, zipname):
    zipf = zipfile.ZipFile(zipname, 'w', zipfile.ZIP_DEFLATED)
    for filepath in files:
        zipf.write(filepath, arcname=os.path.relpath(filepath, start=tmpdir))
    zipf.close()

def delete_files(files):
    for filepath in files:
        os.remove(filepath)

def delete_paths(tmpdir):
    try:
        os.rmdir(os.path.join(tmpdir,PATH_RESDIR))
    except:
        pass
    try:
        os.rmdir(tmpdir)
    except:
        pass

def parse_instances_file(path):
    instances = []
    with open(path) as ifile:
        for line in ifile:
            path, name, args = None, None, None
            if len(line.strip()):
                path, name, args = line.split(",")
            if path == None or name == None:
                continue
            path = path.strip()
            name = name.strip()
            args = args.strip() if type(args) is str else ""
            if os.path.exists(path) and len(name) > 0:
                instances.append((path,name, args))
    return instances

def main(denn,
         instances_file,
         zip_name, 
         tmpdir,
         runs,
         only_statistics,
         delete_files):
    #test zip file
    if os.path.exists(zip_name):
        raise("does not valid zip name")
    #make output dir
    make_output_dir(tmpdir)
    #all tests
    instances = parse_instances_file(instances_file)
    #test 
    if len(instances) == 0:
        raise("does not valid instances file")        
    #results vars
    outputs = {}
    results = {}
    statistics = {}
    times = {}
    times_statistics = {}
    #execute
    for template, name, args in instances:
        outputs[name] = exe_instance(denn, tmpdir, runs, template, name, args, get_only_output=only_statistics)
        #test results
        results[name] = outputs_to_results(outputs[name])
        statistics[name] = mean_var_std(results[name])
        #times results
        times[name] = outputs_to_times(outputs[name])
        times_statistics[name] = mean_var_std(times[name])
    #save
    save_csv_of_dic(outputs, os.path.join(tmpdir,"outputs.csv"))
    save_csv_of_dic(results, os.path.join(tmpdir,"results.csv"))
    save_csv_of_dic(statistics, os.path.join(tmpdir,"statistics.csv"))
    save_csv_of_dic(times, os.path.join(tmpdir,"times.csv"))
    save_csv_of_dic(times_statistics, os.path.join(tmpdir,"times_statistics.csv"))
    #all outputs
    outputs_all = append_all_outputs(outputs)
    outputs_all.extend([os.path.join(tmpdir,CALL_OUTPUT_STD), #stream outputs
                        os.path.join(tmpdir,CALL_OUTPUT_STDERR), #stream outputs
                        os.path.join(tmpdir,"outputs.csv"),
                        os.path.join(tmpdir,"results.csv"),
                        os.path.join(tmpdir,"statistics.csv"),
                        os.path.join(tmpdir,"times.csv"),
                        os.path.join(tmpdir,"times_statistics.csv") #CSVs
                       ])
    #zip
    all_zip_files = copy.deepcopy(outputs_all)
    all_zip_files.extend([template for template,name in instances])
    save_all_in_zipfile(tmpdir, all_zip_files, zip_name)
    #delete
    if delete_files:
        delete_files(outputs_all)
        delete_paths(tmpdir)
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Execute N runs of DENN on templates and datasets')
    parser.add_argument('zip_name', type=str, help='name of output zip')
    parser.add_argument('instances', type=str, help='file of instances runs (default: "{}")'.format(PATH_INSTANCES), nargs='?', default=PATH_INSTANCES)
    parser.add_argument('denn', type=str, help='denn exe path (default: "{}")'.format(PATH_DENN), nargs='?', default=PATH_DENN)
    parser.add_argument('runs', type=int, help='number of runs (default: {})'.format(NRUNS), nargs='?', default=NRUNS)
    parser.add_argument('tmpdir', type=str, help='temporary folder (default: "{}")'.format(PATH_TMPDIR), nargs='?', default=PATH_TMPDIR)
    parser.add_argument('only_statistics', type=bool, help='compute only statistics of last runs (default: false)', nargs='?', default=False)
    parser.add_argument('delete_files', type=bool, help='delete all file at the end of execution (default: true)', nargs='?', default=True)
    args = parser.parse_args()
    main(denn=args.denn, 
         instances_file=args.instances,
         zip_name=args.zip_name,     
         tmpdir=args.tmpdir,
         runs=args.runs,
         only_statistics=args.only_statistics, 
         delete_files=delete_files)

