#include <string>
#include <sstream>

#include "pugixml.hpp"

#include "Representation.h"
#include "AMLException.h"
#include "Event.pb.h"
#include "AML.pb.h"

static const char CAEX_FILE[]                   = "CAEXFile";
static const char INSTANCE_HIERARCHY[]          = "InstanceHierarchy";
static const char ROLE_CLASS_LIB[]              = "RoleClassLib";
static const char ROLE_CLASS[]                  = "RoleClass";
static const char SYSTEM_UNIT_CLASS_LIB[]       = "SystemUnitClassLib";
static const char SYSTEM_UNIT_CLASS[]           = "SystemUnitClass";
static const char INTERNAL_ELEMENT[]            = "InternalElement";
static const char ATTRIBUTE[]                   = "Attribute";
static const char ADDITIONAL_INFORMATION[]      = "AdditionalInformation";

static const char NAME[]                        = "Name";
static const char VALUE[]                       = "Value";
static const char VERSION[]                     = "Version";
static const char ATTRIBUTE_DATA_TYPE[]         = "AttributeDataType";
static const char DESCRIPTION[]                 = "Description";
static const char REF_BASE_SYSTEM_UNIT_PATH[]   = "RefBaseSystemUnitPath";
static const char REF_ROLE_CLASS_PATH[]         = "RefRoleClassPath";
static const char REF_BASE_CLASS_PATH[]         = "RefBaseClassPath";
static const char SUPPORTED_ROLE_CLASS[]        = "SupportedRoleClass";

static const char EDGE_COREDATA[]               = "Edge_CoreData";
static const char EVENT[]                       = "Event";
static const char DATA[]                        = "Data";

static const char KEY_NAME[]                    = "name";
static const char KEY_VALUE[]                   = "value";
static const char KEY_DEVICE[]                  = "device";
static const char KEY_ID[]                      = "id";
static const char KEY_PUSHED[]                  = "pushed";
static const char KEY_CREATED[]                 = "created";
static const char KEY_MODIFIED[]                = "modified";
static const char KEY_ORIGIN[]                  = "origin";

#define IS_NAME(node, name)                     (std::string((node).attribute(NAME).value()) == (name))
#define ADD_VALUE(node, value)                  (node).append_child(VALUE).text().set((value).c_str()) //#TODO: verify non-null after append_child()

#define VERIFY_NON_NULL_THROW_EXCEPTION(var)    if (NULL == (var)) throw AMLException(Exception::NO_MEMORY); 

// for test ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PRINT_NODE(node)    for (pugi::xml_node tool = (node).first_child(); tool; tool = tool.next_sibling()) \
                            {\
                                std::cout << "Tool:";\
                                for (pugi::xml_attribute attr = tool.first_attribute(); attr; attr = attr.next_attribute())\
                                {\
                                     std::cout << " " << attr.name() << "=" << attr.value();\
                                }\
                                std::cout << std::endl;\
                            }\
                            std::cout<<std::endl;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
static void extractAttribute(T* attr, pugi::xml_node xmlNode);

template <class T>
static void extractInternalElement(T* ie, pugi::xml_node xmlNode);

template <class T>
static void extractProtoAttribute(pugi::xml_node xmlNode, T* attr);

template <class T>
static void extractProtoInternalElement(pugi::xml_node xmlNode, T* ie);

template <typename T>
static std::string toString(const T& t)
{
    std::ostringstream ss;
    ss << t;
    return ss.str();
}

class Representation::AMLModel
{
public:
    AMLModel (const std::string& amlFilePath)
    {
        m_doc = new pugi::xml_document();

        pugi::xml_parse_result result = m_doc->load_file(amlFilePath.c_str());
        if (pugi::status_ok != result.status) 
        {
            delete m_doc;
            throw AMLException(Exception::INVALID_FILE_PATH);
        }

        pugi::xml_node xmlCaexFile = m_doc->child(CAEX_FILE);
        if (NULL == xmlCaexFile) 
        {
            delete m_doc;
            throw AMLException(Exception::INVALID_SCHEMA);
        }

        m_roleClassLib = xmlCaexFile.child(ROLE_CLASS_LIB);
        m_systemUnitClassLib = xmlCaexFile.child(SYSTEM_UNIT_CLASS_LIB);
        if (NULL == m_roleClassLib || NULL == m_systemUnitClassLib) 
        {
            delete m_doc;
            throw AMLException(Exception::INVALID_SCHEMA);
        }

        // remove "AdditionalInformation" and "InstanceHierarchy" data
        while (xmlCaexFile.child(ADDITIONAL_INFORMATION))   xmlCaexFile.remove_child(ADDITIONAL_INFORMATION);
        while (xmlCaexFile.child(INSTANCE_HIERARCHY))       xmlCaexFile.remove_child(INSTANCE_HIERARCHY);
    }

    ~AMLModel()
    {
        delete m_doc;
    }

    datamodel::Event* constructEvent(pugi::xml_document* xml_doc)
    {
        assert(nullptr != xml_doc);

        if (NULL == xml_doc->child(CAEX_FILE) ||
            NULL == xml_doc->child(CAEX_FILE).child(INSTANCE_HIERARCHY) ||
            NULL == xml_doc->child(CAEX_FILE).child(INSTANCE_HIERARCHY).child(INTERNAL_ELEMENT))
        {
            throw AMLException(Exception::INVALID_AML_FORMAT);
        }

        pugi::xml_node xml_event = xml_doc->child(CAEX_FILE).child(INSTANCE_HIERARCHY).child(INTERNAL_ELEMENT); // xml_ih might have next_sibling(INSTANCE_HIERARCHY)
        for (; xml_event; xml_event = xml_event.next_sibling(INTERNAL_ELEMENT))
        {
            if (std::string(xml_event.attribute(NAME).value()) == EVENT) break; // What if there are more than 2 events?
        }
        if (NULL == xml_event) 
        {
            throw AMLException(Exception::INVALID_AML_FORMAT);
        }

        datamodel::Event* event = new datamodel::Event();

        for (pugi::xml_node xml_attr = xml_event.child(ATTRIBUTE); xml_attr; xml_attr = xml_attr.next_sibling(ATTRIBUTE))
        {
            if      (IS_NAME(xml_attr, KEY_DEVICE))     event->set_device   (xml_attr.child(VALUE).text().as_string());
            else if (IS_NAME(xml_attr, KEY_ID))         event->set_id       (xml_attr.child(VALUE).text().as_string());
            else if (IS_NAME(xml_attr, KEY_PUSHED))     event->set_pushed   (xml_attr.child(VALUE).text().as_llong());
            else if (IS_NAME(xml_attr, KEY_CREATED))    event->set_created  (xml_attr.child(VALUE).text().as_llong());
            else if (IS_NAME(xml_attr, KEY_MODIFIED))   event->set_modified (xml_attr.child(VALUE).text().as_llong());
            else if (IS_NAME(xml_attr, KEY_ORIGIN))     event->set_origin   (xml_attr.child(VALUE).text().as_llong());
        }

        pugi::xml_node xml_data;
        for (xml_data = xml_event.child(INTERNAL_ELEMENT) ; xml_data; xml_data = xml_data.next_sibling(INTERNAL_ELEMENT))
        {
            if (std::string(xml_data.attribute(NAME).value()) == DATA)
            {
                datamodel::Reading *reading = event->add_reading();

                for (pugi::xml_node xml_attr = xml_data.child(ATTRIBUTE); xml_attr; xml_attr = xml_attr.next_sibling(ATTRIBUTE))
                {
                    if      (IS_NAME(xml_attr, KEY_DEVICE))     reading->set_device   (xml_attr.child(VALUE).text().as_string());
                    else if (IS_NAME(xml_attr, KEY_ID))         reading->set_id       (xml_attr.child(VALUE).text().as_string());
                    else if (IS_NAME(xml_attr, KEY_NAME))       reading->set_name     (xml_attr.child(VALUE).text().as_string());
                    else if (IS_NAME(xml_attr, KEY_VALUE))      reading->set_value    (xml_attr.child(VALUE).text().as_string());
                    else if (IS_NAME(xml_attr, KEY_PUSHED))     reading->set_pushed   (xml_attr.child(VALUE).text().as_llong());
                    else if (IS_NAME(xml_attr, KEY_CREATED))    reading->set_created  (xml_attr.child(VALUE).text().as_llong());
                    else if (IS_NAME(xml_attr, KEY_MODIFIED))   reading->set_modified (xml_attr.child(VALUE).text().as_llong());
                    else if (IS_NAME(xml_attr, KEY_ORIGIN))     reading->set_origin   (xml_attr.child(VALUE).text().as_llong());
                }
            }
        }

        return event;
    }

    pugi::xml_document* constructXmlDoc()
    {
        pugi::xml_document* xml_doc = new pugi::xml_document();

        initializeAML(xml_doc);

        return xml_doc;
    }

    pugi::xml_document* constructXmlDoc(const datamodel::Event* event)
    {
        pugi::xml_document* xml_doc = constructXmlDoc();

        // add InstanceHierarchy
        pugi::xml_node xml_ih = xml_doc->child(CAEX_FILE).append_child(INSTANCE_HIERARCHY);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_ih);

        xml_ih.append_attribute(NAME) = EDGE_COREDATA;

        // add Event as InternalElement
        pugi::xml_node xml_event = addInternalElement(xml_ih, EVENT);
        assert(NULL != xml_event);

        for (pugi::xml_node xml_attr = xml_event.child(ATTRIBUTE); xml_attr; xml_attr = xml_attr.next_sibling(ATTRIBUTE))
        {
            if      (IS_NAME(xml_attr, KEY_DEVICE))     ADD_VALUE(xml_attr, event->device());
            else if (IS_NAME(xml_attr, KEY_ID))         ADD_VALUE(xml_attr, event->id());
            else if (IS_NAME(xml_attr, KEY_PUSHED))     ADD_VALUE(xml_attr, toString(event->pushed()));
            else if (IS_NAME(xml_attr, KEY_CREATED))    ADD_VALUE(xml_attr, toString(event->created()));
            else if (IS_NAME(xml_attr, KEY_MODIFIED))   ADD_VALUE(xml_attr, toString(event->modified()));
            else if (IS_NAME(xml_attr, KEY_ORIGIN))     ADD_VALUE(xml_attr, toString(event->origin()));
        }

        // add Data(s) as InternalElement
        for (int i = 0, size = event->reading_size(); i < size; i++)
        {
            datamodel::Reading reading = event->reading(i);

            pugi::xml_node xml_data = addInternalElement(xml_event, DATA);
            assert(NULL != xml_data);

            for (pugi::xml_node xml_attr = xml_data.child(ATTRIBUTE); xml_attr; xml_attr = xml_attr.next_sibling(ATTRIBUTE))
            {
                if      (IS_NAME(xml_attr, KEY_DEVICE))     ADD_VALUE(xml_attr, reading.device());
                else if (IS_NAME(xml_attr, KEY_NAME))       ADD_VALUE(xml_attr, reading.name());
                else if (IS_NAME(xml_attr, KEY_ID))         ADD_VALUE(xml_attr, reading.id());
                else if (IS_NAME(xml_attr, KEY_VALUE))      ADD_VALUE(xml_attr, reading.value());
                else if (IS_NAME(xml_attr, KEY_PUSHED))     ADD_VALUE(xml_attr, toString(reading.pushed()));
                else if (IS_NAME(xml_attr, KEY_CREATED))    ADD_VALUE(xml_attr, toString(reading.created()));
                else if (IS_NAME(xml_attr, KEY_MODIFIED))   ADD_VALUE(xml_attr, toString(reading.modified()));
                else if (IS_NAME(xml_attr, KEY_ORIGIN))     ADD_VALUE(xml_attr, toString(reading.origin()));
            }
        }

        return xml_doc;
    }

    void appendModel(pugi::xml_document* xml_doc)    
    {
        assert(nullptr != xml_doc);
        // append RoleClassLib and SystemUnitClassLib
        xml_doc->child(CAEX_FILE).append_copy(m_roleClassLib);
        xml_doc->child(CAEX_FILE).append_copy(m_systemUnitClassLib);
    }

private:
    pugi::xml_document* m_doc;
    pugi::xml_node m_roleClassLib;
    pugi::xml_node m_systemUnitClassLib;

    void initializeAML(pugi::xml_document* xml_doc)
    {
        pugi::xml_node xml_decl = xml_doc->prepend_child(pugi::node_declaration);
        xml_decl.append_attribute("version") = "1.0";
        xml_decl.append_attribute("encoding") = "utf-8";
        xml_decl.append_attribute("standalone") = "yes"; // @TODO: required?

        pugi::xml_node xml_caexFile = xml_doc->append_child(CAEX_FILE);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_caexFile);
        xml_caexFile.append_attribute("FileName") = "test.aml"; // @TODO: set by application? or randomly generated? or using time stamp of event
        xml_caexFile.append_attribute("SchemaVersion") = "2.15";
        xml_caexFile.append_attribute("xsi:noNamespaceSchemaLocation") = "CAEX_ClassModel_V2.15.xsd";
        xml_caexFile.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
    }

    pugi::xml_node addInternalElement(pugi::xml_node xml_ih, const std::string suc_name)
    {
        pugi::xml_node xml_ie = xml_ih.append_child(INTERNAL_ELEMENT);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_ie);

        for (pugi::xml_node xml_suc = m_systemUnitClassLib.child(SYSTEM_UNIT_CLASS); xml_suc; xml_suc = xml_suc.next_sibling(SYSTEM_UNIT_CLASS))
        {
            if (std::string(xml_suc.attribute(NAME).value()) == suc_name)
            {
                // set Name
                xml_ie.append_attribute(NAME) = suc_name.c_str();

                // set RefBaseSystemUnitPath
                std::string refBaseSystemUnitPath;
                refBaseSystemUnitPath.append(m_systemUnitClassLib.attribute(NAME).value());
                refBaseSystemUnitPath.append("/");
                refBaseSystemUnitPath.append(suc_name);
                xml_ie.append_attribute(REF_BASE_SYSTEM_UNIT_PATH) = refBaseSystemUnitPath.c_str();

                // set Attribute(s)
                for (pugi::xml_node xml_attr = xml_suc.child(ATTRIBUTE); xml_attr; xml_attr = xml_attr.next_sibling(ATTRIBUTE))
                {
                    xml_ie.append_copy(xml_attr);
                }

                // set SupportedRoleClass
                xml_ie.append_copy(xml_suc.child(SUPPORTED_ROLE_CLASS));

                break;
            }
        }

        return xml_ie;
    }
};

Representation::Representation(const std::string amlFilePath) : m_amlModel (new AMLModel(amlFilePath))
{
}

Representation::~Representation(void)
{
    delete m_amlModel;
}

datamodel::Event* Representation::AmlToEvent(const std::string& xmlStr)
{
    pugi::xml_document dataXml;
    pugi::xml_parse_result result = dataXml.load_string(xmlStr.c_str());
    if (pugi::status_ok != result.status)
    {
        throw AMLException(Exception::INVALID_XML_STR);
    }

    datamodel::Event *event = m_amlModel->constructEvent(&dataXml);
    assert(nullptr != event);
    return event;
}

std::string Representation::EventToAml(const datamodel::Event* event)
{   
    if (nullptr == event)
    {
        throw AMLException(Exception::INVALID_PARAM);
    }

    pugi::xml_document* xml_doc = m_amlModel->constructXmlDoc(event);
    assert(nullptr != xml_doc);
    
    m_amlModel->appendModel(xml_doc);

    std::ostringstream stream;
    xml_doc->save(stream);

    delete xml_doc;

    return stream.str();
}

datamodel::Event* Representation::ByteToEvent(const std::string& byte)
{
    datamodel::CAEXFile* caex = new datamodel::CAEXFile();

    if (false == caex->ParseFromString(byte))
    {
        delete caex;
        throw AMLException(Exception::NOT_IMPL); //@TODO: 'Invalid byte' or 'failed to deserialize' ?
    }

    pugi::xml_document* xml_doc = m_amlModel->constructXmlDoc();
    assert(nullptr != xml_doc);

    // update CAEX attributes
    pugi::xml_node xml_caex = xml_doc->child(CAEX_FILE);
    xml_caex.attribute("FileName")                      = caex->filename().c_str();
    xml_caex.attribute("SchemaVersion")                 = caex->schemaversion().c_str();
    xml_caex.attribute("xsi:noNamespaceSchemaLocation") = caex->xsi().c_str();
    xml_caex.attribute("xmlns:xsi")                     = caex->xmlns().c_str();

    for (datamodel::InstanceHierarchy ih : caex->instancehierarchy())
    {
        pugi::xml_node xml_ih = xml_caex.append_child(INSTANCE_HIERARCHY);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_ih);

        xml_ih.append_attribute(NAME) = ih.name().c_str();

        extractProtoInternalElement(xml_ih, &ih);
    }

    caex->clear_instancehierarchy();
    delete caex;

    datamodel::Event *event = m_amlModel->constructEvent(xml_doc);
    assert(nullptr != event);

    delete xml_doc;

    return event;
}

std::string Representation::EventToByte(const datamodel::Event* event)
{
    if (nullptr == event)
    {
        throw AMLException(Exception::INVALID_PARAM);
    }
    
    // convert Event to XML object
    pugi::xml_document* xml_doc = m_amlModel->constructXmlDoc(event);
    assert(nullptr != xml_doc);

    // convert XML object to AML proto object
    pugi::xml_node xml_caex = xml_doc->child(CAEX_FILE);
    
    datamodel::CAEXFile* caex = new datamodel::CAEXFile();

    caex->set_filename(xml_caex.attribute("FileName").value());
    caex->set_schemaversion(xml_caex.attribute("SchemaVersion").value());
    caex->set_xsi(xml_caex.attribute("xsi:noNamespaceSchemaLocation").value());
    caex->set_xmlns(xml_caex.attribute("xmlns:xsi").value());

    for (pugi::xml_node xml_ih = xml_caex.child(INSTANCE_HIERARCHY); xml_ih; xml_ih = xml_ih.next_sibling(INSTANCE_HIERARCHY))
    {
        datamodel::InstanceHierarchy* ih = caex->add_instancehierarchy();

        ih->set_name    (xml_ih.attribute(NAME).value());
      //ih->set_version (xml_ih.child_value(VERSION)); // @TODO: required?

        extractInternalElement<datamodel::InstanceHierarchy>(ih, xml_ih);
    }

    delete xml_doc;

    std::string binary;
    bool isSuccess = caex->SerializeToString(&binary);

    caex->clear_instancehierarchy();
    delete caex;

    if (false == isSuccess)
    {
        throw AMLException(Exception::NOT_IMPL); //@TODO: 'failed to serialize' ?
    }
    return binary;
}

template <class T>
static void extractProtoAttribute(pugi::xml_node xmlNode, T* attr)
{   
    for (datamodel::Attribute att: attr->attribute()) {
        pugi::xml_node xml_attr = xmlNode.append_child(ATTRIBUTE);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_attr);

        xml_attr.append_attribute(NAME) = att.name().c_str();
        xml_attr.append_attribute(ATTRIBUTE_DATA_TYPE) = att.attributedatatype().c_str();

        extractProtoAttribute(xml_attr, &att);

        xml_attr.append_child(VALUE).text().set(att.value().c_str());
    }

    return;
}

template <class T>
static void extractProtoInternalElement(pugi::xml_node xmlNode, T* ie)
{
    for (datamodel::InternalElement sie: ie->internalelement())
    {
        pugi::xml_node xml_ie = xmlNode.append_child(INTERNAL_ELEMENT);
        VERIFY_NON_NULL_THROW_EXCEPTION(xml_ie);

        xml_ie.append_attribute(NAME) = sie.name().c_str();
        xml_ie.append_attribute(REF_BASE_SYSTEM_UNIT_PATH) = sie.refbasesystemunitpath().c_str();

        extractProtoAttribute(xml_ie, &sie);
        extractProtoInternalElement(xml_ie, &sie);

        if (nullptr != &sie.supportedroleclass())
        {
            if (nullptr != &sie.supportedroleclass().refroleclasspath())
             {
                pugi::xml_node xml_src = xml_ie.append_child(SUPPORTED_ROLE_CLASS);
                VERIFY_NON_NULL_THROW_EXCEPTION(xml_src);

                xml_src.append_attribute(REF_ROLE_CLASS_PATH) = sie.supportedroleclass().refroleclasspath().c_str();
            }
        }
    }
    
    return;
}

template <class T>
static void extractAttribute(T* attr, pugi::xml_node xmlNode)
{
    for (pugi::xml_node xmlAttr = xmlNode.child(ATTRIBUTE); xmlAttr; xmlAttr = xmlAttr.next_sibling(ATTRIBUTE))
    {
        datamodel::Attribute* attr_child = attr->add_attribute();

        attr_child->set_name              (xmlAttr.attribute(NAME).value());
        attr_child->set_attributedatatype (xmlAttr.attribute(ATTRIBUTE_DATA_TYPE).value());
      //attr->set_description       (xmlAttr.child_value(DESCRIPTION)); //@TODO: required?

        extractAttribute<datamodel::Attribute>(attr_child, xmlAttr);

        pugi::xml_node xmlValue = xmlAttr.child(VALUE);
        if (NULL != xmlValue)
        {
            attr_child->set_value(xmlValue.text().as_string());
        }
    }

    return;
}

template <class T>
static void extractInternalElement(T* ie, pugi::xml_node xmlNode)
{
    for (pugi::xml_node xmlIe = xmlNode.child(INTERNAL_ELEMENT); xmlIe; xmlIe = xmlIe.next_sibling(INTERNAL_ELEMENT))    
    {
        datamodel::InternalElement* ie_child = ie->add_internalelement();

        ie_child->set_name                    (xmlIe.attribute(NAME).value());
        ie_child->set_refbasesystemunitpath   (xmlIe.attribute(REF_BASE_SYSTEM_UNIT_PATH).value());
        
        extractAttribute<datamodel::InternalElement>(ie_child, xmlIe);
        extractInternalElement<datamodel::InternalElement>(ie_child, xmlIe);

        pugi::xml_node xmlSrc = xmlIe.child(SUPPORTED_ROLE_CLASS);
        if (NULL != xmlSrc)
        {
            datamodel::SupportedRoleClass* src = new datamodel::SupportedRoleClass();
            src->set_refroleclasspath(xmlSrc.attribute(REF_ROLE_CLASS_PATH).value());

            ie_child->set_allocated_supportedroleclass(src);
        }
    }

    return;
}
