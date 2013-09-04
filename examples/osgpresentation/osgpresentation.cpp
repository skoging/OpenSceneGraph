/* Copyright Robert Osfield, Licensed under the GPL
 *
 * Experimental base for refactor of Present3D
 *
*/

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ScriptEngine>
#include <osg/UserDataContainer>

#include <osgPresentation/Presentation>
#include <osgPresentation/Slide>
#include <osgPresentation/Layer>
#include <osgPresentation/Element>
#include <osgPresentation/Model>
#include <osgPresentation/Volume>
#include <osgPresentation/Image>
#include <osgPresentation/Movie>
#include <osgPresentation/Text>
#include <osgPresentation/Audio>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include <osg/NodeVisitor>

class PrintSupportedProperties : public osgPresentation::Action
{
public:
    PrintSupportedProperties() :
        osgPresentation::Action(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    void apply(osgPresentation::Group& group)
    {
        OSG_NOTICE<<"osgPresentation object : "<<group.className()<<std::endl;
        osgPresentation::PropertyList properties;
        if (group.getSupportedProperties(properties))
        {
            for(osgPresentation::PropertyList::iterator itr = properties.begin();
                itr != properties.end();
                ++itr)
            {
                osgPresentation::ObjectDescription& od = *itr;
                OSG_NOTICE<<"    "<<od.first->className()<<" : "<<od.first->getName()<<", description = "<<od.second<<std::endl;

            }
        }
        traverse(group);
    }
};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    typedef std::list< osg::ref_ptr<osg::Script> > Scripts;
    Scripts scripts;

    std::string filename;
    while(arguments.read("--script",filename))
    {
        osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(filename);
        if (script.valid()) scripts.push_back(script.get());
    }

    // create the model
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if (!model) return 1;

    // assgin script engine to scene graphs
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.python"));
    model->getOrCreateUserDataContainer()->addUserObject(osgDB::readFile<osg::ScriptEngine>("ScriptEngine.js"));

    // assign scripts to scene graph
    for(Scripts::iterator itr = scripts.begin();
        itr != scripts.end();
        ++itr)
    {
       model->addUpdateCallback(new osg::ScriptCallback(itr->get()));
    }

#if 0
    std::string str;
    osg::ref_ptr<osg::ScriptEngine> luaScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.lua");
    if (luaScriptEngine.valid())
    {
        while (arguments.read("--lua", str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                luaScriptEngine->run(script.get());
            }
        }
    }

    osg::ref_ptr<osg::ScriptEngine> v8ScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.V8");
    if (v8ScriptEngine.valid())
    {
        while (arguments.read("--js",str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                v8ScriptEngine->run(script.get());
            }
        }
    }


    osg::ref_ptr<osg::ScriptEngine> pythonScriptEngine = osgDB::readFile<osg::ScriptEngine>("ScriptEngine.python");
    if (pythonScriptEngine.valid())
    {
        while (arguments.read("--python",str))
        {
            osg::ref_ptr<osg::Script> script = osgDB::readFile<osg::Script>(str);
            if (script.valid())
            {
                pythonScriptEngine->run(script.get());
            }
        }
    }
#endif


    osg::ref_ptr<osgPresentation::Presentation> presentation = new osgPresentation::Presentation;
    osg::ref_ptr<osgPresentation::Slide> slide = new osgPresentation::Slide;
    osg::ref_ptr<osgPresentation::Layer> layer = new osgPresentation::Layer;
    osg::ref_ptr<osgPresentation::Group> group = new osgPresentation::Group;
    osg::ref_ptr<osgPresentation::Element> element = new osgPresentation::Element;
    osg::ref_ptr<osgPresentation::Element> text = new osgPresentation::Text;
    presentation->addChild(slide.get());
    slide->addChild(layer.get());
    //layer->addChild(element.get());
    layer->addChild(group.get());
    group->addChild(element.get());
    element->addChild(model.get());
    group->addChild(new osgPresentation::Model);
    group->addChild(text.get());
    group->addChild(new osgPresentation::Audio);
    group->addChild(new osgPresentation::Movie);
    group->addChild(new osgPresentation::Volume);


    text->setProperty("string",std::string("This is a first test"));
    text->setProperty("font",std::string("times.ttf"));
    text->setProperty("character_size",2.2);
    text->setProperty("width",std::string("103.2"));


    PrintSupportedProperties psp;
    presentation->accept(psp);

    osgPresentation::LoadAction load;
    presentation->accept( load );

    viewer.setSceneData( presentation.get() );


    osgDB::writeNodeFile(*presentation, "pres.osgt");

    return viewer.run();
}
