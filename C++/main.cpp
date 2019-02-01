#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <string.h>

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
    int t_lim = 60;
    
    /*
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
            cout << t_lim << endl;
            cout << double(t_lim) << endl;
            InfoSolution info = cuttingplanes(instance,t_lim);
            writeInfos(info,fname3,filename.c_str(),false);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with cuttingplanes plane" << endl;
        }
      }
      closedir (dir);
    }

    
    vector<string> names;
    
    vector<int> l_instance;

    for (int i = 0; i<10; ++i){
        l_instance.push_back(20 +i*20);
    }

    /*for(int i=0; i <16; ++i){
        l_instance.push_back(250 + i*50);
    }*/

    /*for(int i= 0; i < 15; ++i){
        l_instance.push_back(1100 + i*100);
    }

    for (int k = 0; k < l_instance.size(); ++k){
        cout << l_instance[k] << endl;
    }

    vector<char*> end_names;
    end_names.push_back("BAY");
    end_names.push_back("COL");
    end_names.push_back("NY");

    for (int k = 0; k < end_names.size(); ++k){
        for (int l = 0; l < l_instance.size(); ++l){
            stringstream strs;
            strs << l_instance[l];
            string temp_str = strs.str();
            char* length_ = (char*) temp_str.c_str();
            cout << end_names[k] << endl;
            string s("../instances/");
            s.append(length_);
            s.append("_USA-road-d.");
            s.append(end_names[k]);
            s.append(".gr");
            cout << s << endl;
            names.push_back(s);
        }
    }
    cout << "hello1" << endl;
    char* fname = "dualization.csv";

    
    /*if ((dir = opendir ("../instances")) != NULL) {
      // print all the files and directories within directory
      ent = readdir (dir);
      ent = readdir (dir);

      while ((ent = readdir (dir)) != NULL) {
    for (int k = 0; k < names.size(); ++k){
        try{
        //cout << k << endl;
        //cout <<"hello1"<<endl;
        //cout << names[k].c_str() << endl;
        string filename("../instances/");
        filename.append(names[k]);
        //cout << "hello2" << endl;
        cout << filename << endl;
        //filename = names[k];
        Instance instance(filename.c_str(), 0);
        //instance = preprocessInstance(instance);

        // Dualization

        InfoSolution info = dualization(instance,t_lim);
        writeInfos(info,fname,filename.c_str(),false);
        }
        catch(...){
            cout<<"bug in instance"<<ent->d_name<<"with dualization method" << endl;
        }
        
    }
    /*
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
        // Preprocessing
    /*char* fname4 = "dualization.csv";
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
      closedir (dir);/*
    }*/
    Instance instance("../instances/800_USA-road-d.BAY.gr", 0);
    instance = preprocessInstance(instance);
    ShortestCapacitedPath<SemiWorstCaseNodeMetric,SemiWorstCaseEdgeMetric> scp(instance, instance.s, instance.t, SemiWorstCaseNodeMetric(instance), SemiWorstCaseEdgeMetric(instance), false);
    Path path = scp.extractPathNodes(instance.s,instance.t,-1);
    // Dualization with preprocessing
    cout << "with path" << endl;
    //InfoSolution inf = cuttingplanes(instance, 100);
    //cout << "without path" << endl;
    InfoSolution inf2 = cuttingplanes(instance, 100, path);

    //writeInfos(infopreprocess,fname4,filename.c_str(),true);
    
    
    return 0;
}

