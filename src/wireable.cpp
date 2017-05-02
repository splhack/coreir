#include "wireable.hpp"

using namespace std;

///////////////////////////////////////////////////////////
//----------------------- Wireables ----------------------//
///////////////////////////////////////////////////////////


namespace CoreIR {

Select* Wireable::sel(string selStr) {
  Context* c = getContext();
  Type* ret = c->Any();
  Error e;
  bool error = type->sel(c,selStr,&ret,&e);
  if (error) {
    e.message("  Wire: " + toString());
    //e.fatal();
    getContext()->error(e);
  }
  Select* select = moduledef->getCache()->newSelect(moduledef,this,selStr,ret);
  selects.emplace(selStr,select);
  return select;
}

Select* Wireable::sel(uint selStr) { return sel(to_string(selStr)); }

// TODO This might be slow due to insert on a vector. Maybe use Deque?
WirePath Wireable::getPath() {
  Wireable* top = this;
  vector<string> path;
  while(top->isKind(SEL)) {
    Select* s = (Select*) top;
    path.insert(path.begin(), s->getSelStr());
    top = s->getParent();
  }
  if (top->isKind(IFACE)) return {"self",path};
  string instname = ((Instance*) top)->getInstname();
  return {instname, path};
}

Context* Wireable::getContext() { return moduledef->getContext();}

Instance::Instance(ModuleDef* context, string instname, Module* moduleRef, Args configargs) : Wireable(INST,context,nullptr), instname(instname), moduleRef(moduleRef), configargs(configargs), isgen(false), generatorRef(nullptr) {
  assert(moduleRef && "Module is null");
  //TODO checkif instname is unique
  this->type = moduleRef->getType();
}

Instance::Instance(ModuleDef* context, string instname, Generator* generatorRef, Args genargs, Args configargs) : Wireable(INST,context,nullptr), instname(instname), configargs(configargs), isgen(true), generatorRef(generatorRef), genargs(genargs) {
  assert(generatorRef && "Generator is null!");
  this->moduleRef = generatorRef->getModule(genargs);
  this->type = moduleRef->getType();
}

string Interface::toString() const{
  return "self";
}

string Instance::toString() const {
  return instname;
}

//TODO this could throw an error. Bad!
Arg* Instance::getConfigArg(string s) { 
  return configargs.at(s);
}

void Instance::runGenerator() {
  assert(generatorRef && "Not a Generator Instance!");
  
  //If we have already run the generator, do not run again
  if (moduleRef->hasDef()) return;

  //TODO should this be the default behavior?
  //If there is no generatorDef, then just do nothing
  if (!generatorRef->hasDef()) return;
  
  //Actually run the generator
  generatorRef->setModuleDef(moduleRef, genargs);
  assert(moduleRef->hasDef());
}

string Select::toString() const {
  string ret = parent->toString(); 
  if (isNumber(selStr)) return ret + "[" + selStr + "]";
  return ret + "." + selStr;
}

std::ostream& operator<<(ostream& os, const Wireable& i) {
  os << i.toString();
  return os;
}

///////////////////////////////////////////////////////////
//-------------------- SelCache --------------------//
///////////////////////////////////////////////////////////

SelCache::~SelCache() {
  for (auto sel : cache) delete sel.second;
}

Select* SelCache::newSelect(ModuleDef* context, Wireable* parent, string selStr, Type* type) {
  SelectParamType params = {parent,selStr};
  auto it = cache.find(params);
  if (it != cache.end()) {
    assert(it->second->getType() == type);
    return it->second;
  } 
  else {
    Select* s = new Select(context,parent,selStr, type);
    cache.emplace(params,s);
    return s;
  }
}

} //CoreIR namesapce
