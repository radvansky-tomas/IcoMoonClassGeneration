//
//  main.cpp
//  IcomoonClassGenerator
//
//  Created by Tomas Radvansky on 01/10/2015.
//  Copyright © 2015 Radvansky Solutions. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <regex>

using namespace std;

//Public scope enum:
enum ClassType { JAVA = 1, SWIFT = 2, OBJC = 3, UNSELECTED = 0 };

struct Icon
{
    std::string  name;
    std::string  value;
    Icon(std::string n,std::string v)
    {
        name = n;
        value = v;
    }
};

vector<Icon> LoadIcons(std::ifstream* fileRef)
{
    cout << "CSS Class Parsing\n";
    vector<Icon> result;
    //Load Stylesheet
    std::string str((std::istreambuf_iterator<char>(*fileRef)),
                    std::istreambuf_iterator<char>());
    regex integer(".icon-(.+).before..\n.content...(.+)..\n.");
    
    for (std::sregex_iterator it(str.begin(), str.end(), integer), end; it != end; ++it) {
        std::smatch match = *it;
        result.emplace_back(match[1],match[2]);
    }
    return result;
}

string GenerateSwiftClass(vector<Icon>* icons, string* className)
{
    cout << "Swift Class Generation\n";
    
    std::string result = "import UIKit\n";
    //Add vars
    result += "//MARK:Vars\nenum MoonIcons:String {\n";
    //Start icons Enumeration
    for (auto &iconsRef : *icons) // access by reference to avoid copying
    {
        result += "case " + iconsRef.name + " = \"\\u{" + iconsRef.value+ "}\"\n";
    }
    //Add allvalues
    result += "\nstatic let allValues = [";
    int index;
    int iconsCount = icons->size();
    
    cout << "Icons Count:" << iconsCount <<"\n";
    
    for (index=0; index < iconsCount; index++) {
        if (index == iconsCount-1)
        {
           result += icons->at(index).name;
        }
        else
        {
            result += icons->at(index).name + ",\n";
        }
    }
    
    result += "\n}\nclass "+ *className +": NSObject {\nclass func GetMoonIcon(index:Int)->MoonIcons\n{\nreturn MoonIcons.allValues[index]\n}\n\nclass func IndexOfMoonIcon(icon:MoonIcons)->Int\n{\nif let index:Int = find(MoonIcons.allValues,icon)\n{\nreturn index\n}\nelse\n{\nreturn -1\n}\n}\n}";
  
    return result;
}

string GenerateClass(ClassType classType, std::ifstream* fileRef, std::ofstream* classFileRef, string* className)
{
    std::string result;
    vector<Icon> icons = LoadIcons(fileRef);
    switch (classType) {
        case SWIFT:
            result = GenerateSwiftClass(&icons,className);
            break;
            
        default:
            result = "";
            break;
    }
    *classFileRef << result;
    classFileRef->close();
    cout << "'" << *className <<"' has been generated!\n";
    return result;
}

int main(int argc, const char * argv[]) {
    
    //Variables
    std::string stylesheetPath;
    std::string className;
    ClassType selectedType = UNSELECTED;
    // insert code here...
    std::cout << "Icomoon Class Generator v1.0\n";
    std::cout << "Tomas Radvansky (radvansky.tomas@gmail.com)\n";
    
    for (int i = 1; i < argc; ++i) { // Remember argv[0] is the path to the program, we want from argv[1] onwards
        std::string str(argv[i]);
        //Stylesheet parsing
        if (str.compare("-s") == 0)
        {
            if (i+1 <= argc-1)
            {
                stylesheetPath = argv[i+1];
            }
        }
        //Type parsing
        else if (str.compare("-t") == 0)
        {
            if (i+1 <= argc-1)
            {
                std::string str1(argv[i+1]);
                try {
                    int selectedTypeNR = std::stoi(str1);
                    if ((selectedTypeNR>0)&&(selectedTypeNR<4))
                    {
                        selectedType = static_cast<ClassType>(selectedTypeNR);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "'-t' parameter *** error: " << e.what() << '\n' ;
                }
                
            }
        }
        //Path parsing
        else if (str.compare("-n") == 0)
        {
            if (i+1 <= argc-1)
            {
                className = argv[i+1];
            }
        }
    }
    
    //Check if stylesheet exists
    std::ifstream stylesheetStream(stylesheetPath);
    
    if (!stylesheetStream)
    {
        std::cerr << "'" << stylesheetPath <<"' *** error: file does not exist!\n";
        return 0;
    }
    else
    {
        //Prepare path
        string newPath = stylesheetPath.substr(0, stylesheetPath.find_last_of("\\/")) + "/" + className;
        //Add extension
        switch (selectedType) {
            case SWIFT:
                newPath += ".swift";
                break;
            case JAVA:
                newPath += ".java";
                break;
            case OBJC:
                newPath += ".m";
                break;
            default:
                newPath = ".txt";
                break;
        }
        //Check if stylesheet exists
        std::ofstream classStream(newPath);
        
        if (!classStream)
        {
            std::cerr << "'" << newPath <<"' *** error: file does not exist!\n";
            return 0;
        }
        else
        {
            GenerateClass(selectedType, &stylesheetStream, &classStream, &className);
        }
    }
    
    
    return 0;
}
