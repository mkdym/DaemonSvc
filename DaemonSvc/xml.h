#pragma once
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>


//only support utf-8

//class xml_doc;
//class xml_node;

typedef void* xml_doc_ptr;
typedef void* xml_node_ptr;


namespace xml
{
    xml_doc_ptr load_xml_string(const std::string& s);
    std::string get_xml_string(xml_doc_ptr pdoc);
    xml_doc_ptr create_xml();
    void close_xml(xml_doc_ptr pdoc);

    xml_doc_ptr load_xml_file(const std::string& file_path);
    bool save_xml_to_file(xml_doc_ptr pdoc, const std::string& file_path);

    xml_node_ptr get_single_node(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_path);
    void get_node_list(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_path, std::vector<xml_node_ptr>& nodes);

    std::string get_node_value(xml_node_ptr pnode);
    bool get_node_attr(xml_node_ptr pnode, const std::string& attr_name, std::string& attr_value);

    xml_node_ptr append_node(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_path);
    bool remove_node(xml_node_ptr pnode);

    bool set_node_value(xml_node_ptr pnode, const std::string& value);
    bool set_node_attr(xml_node_ptr pnode, const std::string& attr_name, const std::string& attr_value);
    bool remove_node_attr(xml_node_ptr pnode, const std::string& attr_name);

    template<typename Target, typename Source>
    Target any_lexical_cast(const Source& src, const Target& fail_value)
    {
        Target value = fail_value;
        try
        {
            value = boost::lexical_cast<Target>(src);
        }
        catch (boost::bad_lexical_cast& e)
        {
            value = fail_value;
        }
        return value;
    }

    template<typename Target, typename CharType>
    Target string_lexical_cast(const std::basic_string<CharType>& s, const Target& fail_value)
    {
        return any_lexical_cast<Target, std::basic_string<CharType> >(s, fail_value);
    }
}


