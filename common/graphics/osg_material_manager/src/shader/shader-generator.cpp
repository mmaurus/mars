/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "shader-generator.h"

extern "C" {
#include "tsort/tsort.h"
}

#include <configmaps/ConfigData.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>

namespace osg_material_manager {

  using namespace std;
  using namespace configmaps;

  void ShaderGenerator::addShaderFunction(ShaderFunc *func, ShaderType shaderType) {
    map<ShaderType,ShaderFunc*>::iterator it = functions.find(shaderType);
    if(it == functions.end()) {
      functions[shaderType] = func;
    } else {
      it->second->merge( func );
    }
  }

  string ShaderGenerator::generateSource(ShaderType shaderType,
                                         std::string mainSource) {
    map<ShaderType,ShaderFunc*>::iterator it = functions.find(shaderType);
    if(it == functions.end()) {
      return "";
    }
    ShaderFunc *u = it->second;

    stringstream code;
    if(u->getMinVersion() != 0)
      code << "#version " << u->getMinVersion() << endl;
    for(set<string>::iterator it = u->getEnabledExtensions().begin();
        it != u->getEnabledExtensions().end(); ++it)
      code << "#extension " << *it << " : enable" << endl;
    for(set<string>::iterator it = u->getDisabledExtensions().begin();
        it != u->getDisabledExtensions().end(); ++it)
      code << "#extension " << *it << " : disable" << endl;
    code << endl;

    for(set<GLSLUniform>::iterator it = u->getUniforms().begin();
        it != u->getUniforms().end(); ++it)
      code << "uniform " << *it << ";" << endl;

    for(set<GLSLConstant>::iterator it = u->getConstants().begin();
        it != u->getConstants().end(); ++it)
      code << "const " << *it << ";" << endl;

    switch(shaderType)
      {
      case SHADER_TYPE_VERTEX:
        for(set<GLSLVarying>::iterator it = u->getVaryings().begin();
            it != u->getVaryings().end(); ++it)
          code << "varying " << *it << ";" << endl;
        for(set<GLSLAttribute>::iterator it = u->getAttributes().begin();
            it != u->getAttributes().end(); ++it)
          code << "attribute " << *it << ";" << endl;
        break;
      case SHADER_TYPE_FRAGMENT:
        for(set<GLSLVarying>::iterator it = u->getVaryings().begin();
            it != u->getVaryings().end(); ++it)
          code << "varying " << *it << ";" << endl;
        break;
      default: break;
      }
    code << endl;


    std::vector< std::pair<std::string,std::string> > uDeps = u->getDeps();
    for(vector< pair<string,string> >::iterator it = uDeps.begin();
        it != uDeps.end(); ++it)
      code << it->second;

    code << u->generateFunctionCode() << endl;
    if(mainSource.empty()) {
      std::string main = generateMainSource(shaderType);
      return code.str()+main;
    }
    else {
      return code.str()+mainSource;
    }
  }

  string ShaderGenerator::generateMainSource(ShaderType shaderType) {
    map<ShaderType,ShaderFunc*>::iterator it = functions.find(shaderType);
    if(it == functions.end()) {
      return "";
    }
    ShaderFunc *u = it->second;

    stringstream code;
    code << "void main()" << endl;
    code << "{" << endl;

    for(set<GLSLAttribute>::const_iterator it = u->getMainVarDecs().begin();
        it != u->getMainVarDecs().end(); ++it)
      code << "    " << *it << ";" << endl;

    std::priority_queue<PrioritizedLine> lines;
    for(list<MainVar>::const_iterator it = u->getMainVars().begin();
        it != u->getMainVars().end(); ++it)
      lines.push(PrioritizedLine((*it).toString(), (*it).priority, lines.size()));
      //code << "    " << (*it).name << " = " << (*it).value<< ";" << endl;

    for(vector<FunctionCall>::const_iterator it = u->getFunctionCalls().begin(); it != u->getFunctionCalls().end(); ++it)
      lines.push(PrioritizedLine((*it).toString(), (*it).priority, lines.size()));
      //code << "    " << *it << endl;

    for(vector<PrioritizedLine>::const_iterator it = u->getSnippets().begin(); it != u->getSnippets().end(); ++it) {
      lines.push(*it);
    }

    while(!lines.empty()) {
      //cout << "Line: " << lines.top().line << "with PRIO: " << lines.top().priority << endl;
      code << "    " << lines.top().line << ";" << " //Priority: " << lines.top().priority << endl;
      lines.pop();
    }

    for(vector<GLSLExport>::const_iterator it = u->getExports().begin();
        it != u->getExports().end(); ++it)
      code << "    " << *it << ";" << endl;
    for(set<GLSLSuffix>::iterator it = u->getSuffixes().begin();
        it != u->getSuffixes().end(); ++it)
      code << "    " << *it << ";" << endl;

    code << "}" << endl;

    return code.str();
  }

  static void printSource(const string &source) {
    std::string line;
    std::istringstream stream(source);
    int i = 0;
    while (stream.good()) {
      std::getline(stream, line);
      std::cerr << i++ << line << std::endl;
    }
  }

  osg::Program* ShaderGenerator::generate() {
    osg::Program *prog = new osg::Program();

    if(functions.find(SHADER_TYPE_GEOMETRY) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::GEOMETRY);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_GEOMETRY) );
    }
    if(functions.find(SHADER_TYPE_VERTEX) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::VERTEX);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_VERTEX) );
      //printSource( shader->getShaderSource() );
    }
    if(functions.find(SHADER_TYPE_FRAGMENT) != functions.end()) {
      osg::Shader *shader = new osg::Shader(osg::Shader::FRAGMENT);
      prog->addShader(shader);
      shader->setShaderSource( generateSource(SHADER_TYPE_FRAGMENT) );
      //printSource( shader->getShaderSource() );
    }

    return prog;
  }

  void ShaderGenerator::loadGraphShader(const std::string &filename, const std::string &resPath) {
    stringstream code;
    std::map<unsigned long, ConfigMap> nodeMap;
    std::vector<ConfigMap*> sortedNodes;
    std::map<std::string, unsigned long> nodeNameId;
    std::map<unsigned long, ConfigMap>::iterator nodeIt;
    std::vector<ConfigMap*>::iterator sNodeIt;
    std::vector<std::string> add; // lines to add after function call generation
    ConfigMap model = ConfigMap::fromYamlFile(filename);
    ConfigMap graph = ConfigMap::fromYamlString(model["versions"][0]["components"].getString());
    ConfigMap filterMap;
    filterMap["int"] = 1;
    filterMap["float"] = 1;
    filterMap["vec2"] = 1;
    filterMap["vec3"] = 1;
    filterMap["vec4"] = 1;
    filterMap["sampler2D"] = 1;
    filterMap["outColor"] = 1;

    ConfigVector::iterator it, et;
    // create node ids for tsort
    unsigned long id = 1;
    for(it=graph["nodes"].begin(); it!=graph["nodes"].end(); ++it) {
      std::string function = (*it)["model"]["name"];
      if(!filterMap.hasKey(function)) {
        nodeMap[id] = (*it);
        nodeNameId[(*it)["name"].getString()] = id++;
      }
    }

    // create relations for tsort
    for(et=graph["edges"].begin(); et!=graph["edges"].end(); ++et) {
      std::string from = (*et)["from"]["name"];
      std::string to = (*et)["to"]["name"];
      if(nodeNameId.find(from) != nodeNameId.end() &&
         nodeNameId.find(to) != nodeNameId.end()) {
        add_relation(nodeNameId[from], nodeNameId[to]);
      }
    }
    tsort();
    unsigned long *ids = get_sorted_ids();
    while(*ids != 0) {
      sortedNodes.push_back(&(nodeMap[*ids]));
      ids++;
    }
    for(nodeIt=nodeMap.begin(); nodeIt!=nodeMap.end(); ++nodeIt) {
      if(find(sortedNodes.begin(), sortedNodes.end(), &(nodeIt->second)) == sortedNodes.end()) {
        sortedNodes.push_back(&(nodeIt->second));
      }
    }

    code << "void main() {" << endl;

    // create edge variables
    for(et=graph["edges"].begin(); et!=graph["edges"].end(); ++et) {
      std::string dataType = (*et)["data"]["dataType"];
      std::string name = (*et)["name"];
      if(isdigit(name[0])) {
        name = "e" + name;
        (*et)["name"] = name;
      }
      std::string from = (*et)["from"]["name"];
      std::string to = (*et)["to"]["name"];
      bool print = true;
      for(it=graph["nodes"].begin(); it!=graph["nodes"].end(); ++it) {
        if((*it)["name"].getString() == from) {
          if(filterMap.hasKey((*it)["model"]["name"].getString())) {
            (*et)["name"] = (*it)["name"];
            print = false;
          }
        }
        if((*it)["name"].getString() == to) {
          if((*it)["model"]["name"].getString() == "outColor") {
            std::string t = "  gl_FragColor = " + (*et)["name"].getString() + ";\n";
            add.push_back(t);
          }
        }
      }
      if(print) {
        code << "  " << dataType << " " << name << endl;
      }
    }
    code << endl;

    // create function calls
    for(sNodeIt=sortedNodes.begin(); sNodeIt!=sortedNodes.end(); ++sNodeIt) {
      ConfigMap &nodeMap = **sNodeIt;
      std::string function = nodeMap["model"]["name"];
      if(!filterMap.hasKey(function)) {
        // todo: make shader-type sensitive!
        ConfigMap functionInfo = ConfigMap::fromYamlFile(resPath+"/graph_shader/"+function+".yaml");
        code << "  " << function << "(";
        std::priority_queue<std::pair<int, std::string> > incoming, outgoing;
        bool first = true;
        // search for incoming and outgoing edges
        // todo: handle index of function value
        /* todo: add default values
         *       incoming.resize(numInputs, default);
         *       outgoing.resize(numOutputs);
         *       create variables for not connected outputs
         */

        for(et=graph["edges"].begin(); et!=graph["edges"].end(); ++et) {
          std::string paramName = (*et)["name"];
          if((*et)["to"]["name"].getString() == nodeMap["name"].getString()) {
            incoming.push(std::make_pair((int)functionInfo["params"]["in"][paramName], paramName));
          }
          else if((*et)["from"]["name"].getString() == nodeMap["name"].getString()) {
            outgoing.push(std::make_pair((int)functionInfo["params"]["out"][paramName], paramName));
          }
        }
        while (!incoming.empty()) {
          if(!first) {
            code << ", ";
          }
          first = false;
          code << incoming.top().second;
          incoming.pop();
        }
        while (!outgoing.empty()) {
          if(!first) {
            code << ", ";
          }
          first = false;
          code << outgoing.top().second;
          outgoing.pop();
        }
        // search for outgoing edges
        code << ");" << endl;
      }
    }
    for(size_t i=0; i<add.size(); ++i) {
      code << add[i];
    }
    code << "}" << endl;
    fprintf(stderr, "\n%s\n", code.str().c_str());
  }

} // end of namespace osg_material_manager
