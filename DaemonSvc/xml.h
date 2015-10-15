#pragma once
#include <string>
#include <vector>


//only support utf-8

//class xml_doc;
//class xml_node;

typedef void* xml_doc_ptr;
typedef void* xml_node_ptr;


namespace xml
{
    //if parse error, return NULL
    //should call close_xml on returned xml ptr to release resource when no longer needed
    xml_doc_ptr load_xml_string(const std::string& s);

    //get all string of the xml doc
    std::string get_xml_string(const xml_doc_ptr pdoc);

    //if create fail, return NULL
    //always version=1.0, encoding=utf-8
    //should call close_xml on returned xml ptr to release resource when no longer needed
    xml_doc_ptr create_xml();

    void close_xml(xml_doc_ptr pdoc);

    //if parse error, return NULL
    //should call close_xml on returned xml ptr to release resource when no longer needed
    //have disabled wow64 fs redirection in function
    xml_doc_ptr load_xml_file(const std::string& file_path);
    bool save_xml_to_file(const xml_doc_ptr pdoc, const std::string& file_path);

    //if pparent_node != NULL, use it as parant node, otherwise, use pdoc as parant node
    //you should ensure at least one of the two ptrs is valid
    //node path can contains "/" as path separator
    //if path is not found, return NULL
    xml_node_ptr get_single_node(const xml_doc_ptr pdoc, const xml_node_ptr pparent_node, const std::string& node_path);

    //see comments on get_single_node
    //"nodes" will be empty when path not found
    void get_node_list(const xml_doc_ptr pdoc,
        const xml_node_ptr pparent_node,
        const std::string& node_path,
        std::vector<xml_node_ptr>& nodes);

    std::string get_node_value(const xml_node_ptr pnode);

    //if attr not found, return false, otherwise return true
    //attr_value will store the attr value on return
    bool get_node_attr(const xml_node_ptr pnode, const std::string& attr_name, std::string& attr_value);

    //see comments on get_single_node
    //node name must be a name, should not contain "/"
    //return appended node ptr
    xml_node_ptr append_node(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_name);

    bool remove_node(xml_node_ptr pnode);

    //will remove current value firstly if has, then set the new value
    //you can pass a empty string to "value", in order to remove the value
    //when "cdata" is true, will add a CDATA format value, see http://www.w3school.com.cn/xml/xml_cdata.asp
    bool set_node_value(xml_node_ptr pnode, const std::string& value, const bool cdata = false);

    //will remove same name attr firstly if has, then add the new attr
    bool set_node_attr(xml_node_ptr pnode, const std::string& attr_name, const std::string& attr_value);

    //remove specified attr if has
    bool remove_node_attr(xml_node_ptr pnode, const std::string& attr_name);
}


