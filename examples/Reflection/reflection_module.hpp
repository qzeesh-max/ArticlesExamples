#ifndef _REFLECTION_MODULE
#define _REFLECTION_MODULE

namespace reflection
{
typedef map < string, boost::shared_ptr<IClass> > ClassMap_t;

typedef ClassMap_t::const_iterator ClassMapConstIterator;

class Module
{
protected:
    string ModuleName;
    ClassMap_t Classes;
public:
    Module (const string& moduleName):
        ModuleName (moduleName) { }
    const string& getModuleName () const
    {
        return ModuleName;
    }
    ClassMapConstIterator beginClasses () const
    {
        return Classes. begin ();
    }
    ClassMapConstIterator endClasses () const
    {
        return Classes. end ();
    }
    IClass* getClass (const string& className)
    {
        ClassMapConstIterator it = Classes.find (className);

        if (it != Classes.end ()) 
		return it->second.get();

        throw logic_error (("Class is not part of module: " + ModuleName).c_str ());
    }
    template < typename T > IClass* createClass (const string& className)
    {
        return (Classes[className] = boost::shared_ptr<IClass>(new Class < T > (className))).get();
    }
};

};

#endif
