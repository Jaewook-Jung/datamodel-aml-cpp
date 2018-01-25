#include <iostream>
#include <string>

#include "Event.pb.h"
#include "AML.pb.h"

using namespace std;

// datamodel::Event getProtoBufEvent()
// {
//     datamodel::Event event;
//     event.set_device("device");
//     event.set_created(10);
//     event.set_modified(20);
//     event.set_id("id");
//     event.set_pushed(10);
//     event.set_origin(20);

//     datamodel::Reading *reading1 = event.add_reading();
//     reading1->set_name("reading1");
//     reading1->set_value("10");
//     reading1->set_created(25);
//     reading1->set_device("device");
//     reading1->set_modified(20);
//     reading1->set_id("id1");
//     reading1->set_origin(25);
//     reading1->set_pushed(1);

//     datamodel::Reading *reading2 = event.add_reading();
//     reading2->set_name("reading2");
//     reading2->set_value("20");
//     reading2->set_created(30);
//     reading2->set_device("device");
//     reading2->set_modified(20);
//     reading2->set_id("id2");
//     reading2->set_origin(25);
//     reading2->set_pushed(1);

//     return event;
// }

// void toProtoAtt(datamodel::Attribute *protoatt, Attribute *att) {
//     protoatt->set_name(att->getName());
//     protoatt->set_attributedatatype(att->getAttributeDataType());
//     protoatt->set_value(att->getValue());

    
//     for(Attribute *satt: att->getAttribute()) {
//         datamodel::Attribute *subprotoatt = protoatt->add_attribute();
//         toProtoAtt(subprotoatt, satt);
//     }

//     return;
// }

// void toProtoIe(datamodel::InternalElement *protoie, InternalElement *ie){
//     protoie->set_name(ie->getName());
//     protoie->set_refbasesystemunitpath(ie->getRefBaseSystemUnitPath());
    
//     datamodel::SupportedRoleClass *src;
//     src->set_refroleclasspath(ie->getSupportedRoleClass()->getRefRoleClassPath());
//     protoie->set_allocated_supportedroleclass(src);
    
//     for(InternalElement *subie: ie->getInternalElement()) {
//         datamodel::InternalElement *subprotoie = protoie->add_internalelement();
//         toProtoIe(subprotoie, subie);
//     }   

//     for(Attribute *att: ie->getAttribute()) {
//         datamodel::Attribute *protoatt = protoie->add_attribute();
//         toProtoAtt(protoatt, att);
//     }

//     return;
// }

// std::string toProtoBuf(InstanceHierarchy* ih) {
//     if(ih == nullptr) {
//         cout << ("ih == nullptr") << endl;
//         return nullptr;
//     }

//     datamodel::InstanceHierarchy protoih;
    
//     protoih.set_name(ih->getName());
//     protoih.set_version(ih->getVersion());

//     for(InternalElement *ie: ih->getInternalElement()) {
//         datamodel::InternalElement *protoie = protoih.add_internalelement();
//         toProtoIe(protoie, ie);
//     }

//     if(&protoih == nullptr) {
//         return nullptr;
//     }
    
//     std::string ret;
//     bool chk = protoih.SerializeToString(&ret);
    
//     if(chk == false) {
//         cout << "serialize err" << endl;
//     }
    
//     return ret;
// }   

// Attribute* toAMLAtt(datamodel::Attribute *protoatt) {
//     Attribute* att = new Attribute();
//     att->setName(protoatt->name());
//     att->setAttributeDataType(protoatt->attributedatatype());
//     att->setValue(protoatt->value());

//     for(datamodel::Attribute subprotoatt: protoatt->attribute()) {
//         Attribute *subatt = toAMLAtt(&subprotoatt);
//         att->getAttribute().push_back(subatt);
//     }

//     return att;
// }

// InternalElement* toAMLIe(datamodel::InternalElement *protoie) {
//     InternalElement* ie = new InternalElement();
//     ie->setName(protoie->name());
//     ie->setRefBaseSystemUnitPath(protoie->refbasesystemunitpath());

//     SupportedRoleClass *src = new SupportedRoleClass();
//     src->setRefRoleClassPath(protoie->supportedroleclass().refroleclasspath());

//     for(datamodel::InternalElement subprotoie: protoie->internalelement()) {
//         InternalElement *subie = toAMLIe(&subprotoie);
//         ie->getInternalElement().push_back(subie);
//     }

//     for(datamodel::Attribute protoatt: protoie->attribute()) {
//         Attribute *att = toAMLAtt(&protoatt);
//         ie->getAttribute().push_back(att);
//     }

//     return ie;
// }

// AMLObject* toAMLObj(std::string str) {
//     datamodel::InstanceHierarchy protoih;
//     protoih.ParseFromString(str);

//     AMLObject* ret = new AMLObject();

//     InstanceHierarchy* ih = new InstanceHierarchy();
//     ih->setName(protoih.name());
//     ih->setVersion(protoih.version());

//     for(datamodel::InternalElement protoie: protoih.internalelement()) {
//         InternalElement* ie = toAMLIe(&protoie);
//         ih->getInternalElement().push_back(ie);
//     }

//     ret->setInstanceHierarchy(ih);

//     return ret;
// }


int main() {

    // InternalElement* testie = new class::InternalElement("test");
    // testie->addSubInternalElement("sub1");
    // testie->addSubInternalElement("sub2");
    // testie->addSubInternalElement("sub3");

    // InternalElement* sub1 = testie->searchInternalElement("sub1");
    // testie->searchInternalElement("sub4");

    // datamodel::Event sample_event = getProtoBufEvent();

    // cout << "Event" << endl;
    // cout << "Event device : " << sample_event.device() << endl;
    // cout << "Event pushed : " << sample_event.pushed() << endl;
    // cout << "Event id : " << sample_event.id() << endl;
    // cout << "Event created : " << sample_event.created() << endl;
    // cout << "Event modified : " << sample_event.modified() << endl;
    // cout << "Event origin : " << sample_event.origin() << endl;

    // int size = sample_event.reading_size();
    // int i = 0;
    // while (i < size) {
    //     datamodel::Reading reading = sample_event.reading(i++);
    //     cout << "Data" << endl;
    //     cout << "Data device : " << reading.device() << endl;
    //     cout << "Data pushed : " << reading.pushed() << endl;
    //     cout << "Data name : " << reading.name() << endl;
    //     cout << "Data value : " << reading.value() << endl;
    //     cout << "Data id : " << reading.id() << endl;
    //     cout << "Data created : " << reading.created() << endl;
    //     cout << "Data modified : " << reading.modified() << endl;
    //     cout << "Data origin : " << reading.origin() << endl;
    // }
    // cout<<"11111111111111111111"<<endl;

    // // datamodel::InstanceHierarchy ih;
    // // ih.set_name("test");
    // // ih.set_version("1.1.1");

    // // cout << "InstanceHierarchy" << endl;
    // // cout << "InstanceHierarchy name : " << ih.name() << endl;
    // // cout << "InstanceHierarchy version : " << ih.version() << endl;

    // InstanceHierarchy* ih = new InstanceHierarchy();
    // ih->setName("test");
    // ih->setVersion("1.1.1");
    // cout << "InstanceHierarchy" << endl;
    // cout << "InstanceHierarchy name : " << ih->getName() << endl;
    // cout << "InstanceHierarchy version : " << ih->getVersion() << endl;

    // std::string teststr;
    // //bool result = ih.SerializeToString(&teststr);
    // teststr = toProtoBuf(ih);
    // // if(result == false) {
    // //     cout<<"error"<<endl;
    // // }

    // cout << teststr << endl;

    // // datamodel::InstanceHierarchy ih2;
    
    // // ih2.ParseFromString(teststr);
    // cout<<"11111111111111111111"<<endl;
    // AMLObject* obj = toAMLObj(teststr);
    // InstanceHierarchy* ih2 = obj->getInstanceHierarchy();

    // cout << "InstanceHierarchy" << endl;
    // cout << "InstanceHierarchy name : " << ih2->getName() << endl;
    // cout << "InstanceHierarchy version : " << ih2->getVersion() << endl;

     return 0;

}