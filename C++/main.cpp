#include <iostream>
#include <fstream>
#include <stdio.h>
#include "instance.h"
#include "path.h"
#include "sorted_list.h"
#include "shortest_capacited_path.h"
#include "preprocess_nodes.h"
#include "cplexsolvings.h"
#include "infosolution.h"

#include <sys/time.h>
#include <dirent.h>

using namespace std;

bool isFile(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


const std::string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void writeInfos(const InfoSolution& info,const char* namefile, const char* instanceName, const bool preprocess)
{
    bool nofile = !isFile(namefile);
    std::ofstream myfile;
    myfile.open (namefile, std::ios_base::app);
    if (nofile)
        myfile << "instance_name,solution,bound,time,time_before_closing_model,is_optimal,method,date,preprocess\n";
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    myfile<< instanceName << ","<<info.solution<<","<<info.bestbound<<","<<info.t<<","<<info.tWithoutClosing<<","<<info.optimal<<","<<info.method<<","<<currentDateTime()<<","<<preprocess<<"\n";
    myfile.close();
}

int main() {
    double t;
    double elapsedTime;
    double preprocessingTime;

    // InfoSolution info;

    DIR *dir;
    struct dirent *ent;
    int t_lim = 900;
    char* fname = "dualization.csv";

    if ((dir = opendir ("../instances")) != NULL) {
      // print all the files and directories within directory
      ent = readdir (dir);
      ent = readdir (dir);

      while ((ent = readdir (dir)) != NULL) {
        try{

        string filename("../instances/");
        filename.append(ent->d_name);
        cout << filename << endl;
        Instance instance(filename.c_str(), 0);

        // Dualization
        InfoSolution info = dualization(instance,t_lim);
        writeInfos(info,fname,filename.c_str(),false);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with dualization method" << endl;
        }
      }
      closedir (dir);
    }

    char* fname2 = "branchandcut.csv";
    if ((dir = opendir ("../instances")) != NULL) {
      // print all the files and directories within directory
      ent = readdir (dir);
      ent = readdir (dir);
      while ((ent = readdir (dir)) != NULL) {
        try{
            string filename("../instances/");
            filename.append(ent->d_name);
            cout << filename << endl;
            Instance instance(filename.c_str(), 0);

            // Dualization
            InfoSolution info = branchandcut(instance,t_lim);
            writeInfos(info,fname2,filename.c_str(),false);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with branch and cut method" << endl;
        }
      }
      closedir (dir);
    }

    char* fname3 = "cuttingplanes.csv";
    if ((dir = opendir ("../instances")) != NULL) {
      // print all the files and directories within directory
      ent = readdir (dir);
      ent = readdir (dir);
      while ((ent = readdir (dir)) != NULL) {
        try{
            string filename("../instances/");
            filename.append(ent->d_name);
            cout << filename << endl;
            Instance instance(filename.c_str(), 0);

            // Dualization
            InfoSolution info = cuttingplanes(instance,t_lim);
            writeInfos(info,fname3,filename.c_str(),false);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with cuttingplanes plane" << endl;
        }
      }
      closedir (dir);
    }

        // Branch and cut
        /*t = get_time();
        //info = branchandcut(instance,timeLimit...)
        elapsedTime = get_time()-t;
        // writeInfos(info,elapsedTime...)

        // Cuttingplane
        t = get_time();
        //info = cuttingplane(instance,timeLimit...)
        elapsedTime = get_time()-t;
        // writeInfos(info,elapsedTime...)*/

        // writeInfos(info,elapsedTime...)

        // Preprocessing
    char* fname4 = "dualization.csv";
    if ((dir = opendir ("../instances")) != NULL) {
      // print all the files and directories within directory
      ent = readdir (dir);
      ent = readdir (dir);
      while ((ent = readdir (dir)) != NULL) {
        try{
            string filename("../instances/");
            filename.append(ent->d_name);
            cout << filename << endl;
            Instance instance(filename.c_str(), 0);
            instance = preprocessInstance(instance);
            //ShortestCapacitedPath<SemiWorstCaseNodeMetric,SemiWorstCaseEdgeMetric> scp(instance, instance.s, instance.t, SemiWorstCaseNodeMetric(instance), SemiWorstCaseEdgeMetric(instance), false);
            //Path path = scp.extractPathNodes(instance.s,instance.t,-1);
            // Dualization with preprocessing
            InfoSolution infopreprocess = dualization(instance,t_lim);//path);
            writeInfos(infopreprocess,fname4,filename.c_str(),true);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with dualization and preprocess" << endl;
        }
      }
      closedir (dir);
    }
        
        
        // writeInfos(info,elapsedTime...)*/
	return 0;
}

