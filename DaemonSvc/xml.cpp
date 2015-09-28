#include <cassert> //for assert
#include <fstream> //for ofstream in save_xml_to_file
#include <iterator> //for std::back_inserter
#include <boost/smart_ptr/scoped_ptr.hpp> //for scoped_ptr in load_xml_file
#include "../rapidxml-1.13/rapidxml.hpp"
#include "../rapidxml-1.13/rapidxml_utils.hpp" //for rapidxml::file in load_xml_file
#include "../rapidxml-1.13/rapidxml_print.hpp" //for rapidxml::print in get_xml_string
#include "boost_algorithm_string.h" //for boost::algorithm::split in get_single_node
#include "logger.h"
#include "xml.h"


typedef rapidxml::xml_document<char> xml_doc;
typedef rapidxml::xml_node<char> xml_node;
typedef rapidxml::xml_attribute<char> xml_attr;


static xml_doc* xml_doc_cast(xml_doc_ptr p)
{
    return reinterpret_cast<xml_doc *>(p);
}

static xml_node* xml_node_cast(xml_node_ptr p)
{
    return reinterpret_cast<xml_node *>(p);
}


xml_doc_ptr xml::load_xml_string(const std::string& s)
{
    xml_doc* pdoc = new xml_doc();
    char* doc_str = pdoc->allocate_string(s.c_str());

    bool has_error = true;
    try
    {
        pdoc->parse<rapidxml::parse_full>(doc_str);
        has_error = false;
    }
    catch (rapidxml::parse_error& e)
    {
        ErrorLogA("parse xml string fail, error: %s", e.what());
    }

    if (has_error)
    {
        delete pdoc;
        pdoc = NULL;
    }
    return pdoc;
}

std::string xml::get_xml_string(xml_doc_ptr pdoc)
{
    assert(pdoc);

    std::string s;
    rapidxml::print(std::back_inserter(s), *(xml_doc_cast(pdoc)), 0);
    return s;
}

//hard-code: 1.0 utf-8
xml_doc_ptr xml::create_xml()
{
    xml_doc* pdoc = new xml_doc();
    xml_node* pdeclaration = pdoc->allocate_node(rapidxml::node_declaration);
    pdoc->append_node(pdeclaration);

    xml_attr* pattr_ver = pdoc->allocate_attribute("version", "1.0");
    pdeclaration->append_attribute(pattr_ver);

    xml_attr* pattr_enc = pdoc->allocate_attribute("encoding", "utf-8");
    pdeclaration->append_attribute(pattr_enc);

    return pdoc;
}

void xml::close_xml(xml_doc_ptr pdoc)
{
    delete xml_doc_cast(pdoc);
    pdoc = NULL;
}

//todo: wow64
xml_doc_ptr xml::load_xml_file(const std::string& file_path)
{
    bool has_error = true;
    boost::scoped_ptr<rapidxml::file<char> > pf;
    try
    {
        pf.reset(new rapidxml::file<char>(file_path.c_str()));
        has_error = false;
    }
    catch (std::runtime_error&)
    {
        ErrorLogA("can not open file: %s", file_path.c_str());
    }

    if (has_error)
    {
        return NULL;
    }
    else
    {
        return load_xml_string(pf->data());
    }
}

//todo: wow64
bool xml::save_xml_to_file(xml_doc_ptr pdoc, const std::string& file_path)
{
    assert(pdoc && !file_path.empty());

    bool has_error = true;
    try
    {
        std::ofstream f;
        f.open(file_path.c_str(), std::ofstream::out | std::ofstream::trunc);
        if (!f.is_open())
        {
            ErrorLogA("can not open file[%s]", file_path.c_str());
        }
        else
        {
            std::string s = get_xml_string(pdoc);
            f.write(s.c_str(), s.size());
            has_error = false;
        }
    }
    catch (std::ofstream::failure& e)
    {
        ErrorLogA("can not open or write file[%s], error: %s", file_path.c_str(), e.what());
    }

    if (has_error)
    {
        return false;
    }
    else
    {
        return true;
    }
}

xml_node_ptr xml::get_single_node(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_path)
{
    assert(pdoc || pparent_node);

    std::vector<std::string> name_levels;
    boost::algorithm::split(name_levels, node_path, boost::algorithm::is_any_of("/"));

    std::vector<std::string>::const_iterator iter_name = name_levels.begin();
    xml_node* pchild = xml_node_cast(pparent_node ? pparent_node : pdoc);

    std::string find_path;
    while (iter_name != name_levels.end() && pchild)
    {
        find_path += "/" + *iter_name;

        pchild = pchild->first_node(iter_name->c_str(), 0, false);
        ++iter_name;
    }

    if (iter_name == name_levels.end() && pchild)
    {
        return pchild;
    }
    else
    {
        ErrorLogA("can not find node path[%s]", find_path.c_str());
        return NULL;
    }
}

void xml::get_node_list(xml_doc_ptr pdoc,
                        xml_node_ptr pparent_node,
                        const std::string& node_path,
                        std::vector<xml_node_ptr>& nodes)
{
    assert(pdoc || pparent_node);

    nodes.clear();
    xml_node* pparent_level_node = NULL;

    std::string last_level_name;
    size_t parent_level_pos = node_path.find_last_of('/');
    if (std::string::npos == parent_level_pos)//only one level
    {
        last_level_name = node_path;
        pparent_level_node = xml_node_cast(pparent_node ? pparent_node : pdoc);
    }
    else
    {
        std::string parent_level_path = node_path.substr(0, parent_level_pos);
        last_level_name = node_path.substr(parent_level_pos + 1);
        pparent_level_node = xml_node_cast(get_single_node(pdoc, pparent_node, parent_level_path));
    }

    if (pparent_level_node)
    {
        for (xml_node* psibling_node = pparent_level_node->first_node(last_level_name.c_str(), 0, false);
            psibling_node;
            psibling_node = psibling_node->next_sibling(last_level_name.c_str(), 0, false))
        {
            nodes.push_back(psibling_node);
        }
    }
}

std::string xml::get_node_value(xml_node_ptr pnode)
{
    assert(pnode);

    std::string s;
    const xml_node* pvalue_node = xml_node_cast(pnode)->first_node();
    if (pvalue_node)
    {
        s = pvalue_node->value();
    }
    return s;
}

bool xml::get_node_attr(xml_node_ptr pnode, const std::string& attr_name, std::string& attr_value)
{
    assert(pnode);

    const xml_attr* pattr = xml_node_cast(pnode)->first_attribute(attr_name.c_str(), 0, false);
    if (pattr)
    {
        attr_value = pattr->value();
        return true;
    }
    else
    {
        attr_value.clear();
        return false;
    }
}

xml_node_ptr xml::append_node(xml_doc_ptr pdoc, xml_node_ptr pparent_node, const std::string& node_name)
{
    assert(pdoc || pparent_node);
    assert(!node_name.empty());

    xml_node* preal_parent = xml_node_cast(pparent_node ? pparent_node : pdoc);
    char* pname = preal_parent->document()->allocate_string(node_name.c_str());
    xml_node* pchild = preal_parent->document()->allocate_node(rapidxml::node_element, pname);
    preal_parent->append_node(pchild);

    return pchild;
}

bool xml::remove_node(xml_node_ptr pnode)
{
    assert(pnode);

    xml_node* preal_node = xml_node_cast(pnode);
    preal_node->document()->remove_node(preal_node);
    return true;
}

//value type: data or cdata
bool xml::set_node_value(xml_node_ptr pnode, const std::string& value, const bool cdata /*= false*/)
{
    assert(pnode);

    xml_node* preal_node = xml_node_cast(pnode);

    //remove current data or cdata child nodes
    std::vector<xml_node*> remove_nodes;
    for (xml_node* pchild_node = preal_node->first_node(NULL, 0, false);
        pchild_node;
        pchild_node = pchild_node->next_sibling(NULL, 0, false))
    {
        if (pchild_node->type() == rapidxml::node_cdata
            || pchild_node->type() == rapidxml::node_data)
        {
            remove_nodes.push_back(pchild_node);
        }
    }

    for (std::vector<xml_node*>::iterator iter_ndoe = remove_nodes.begin();
        iter_ndoe != remove_nodes.end();
        ++iter_ndoe)
    {
        preal_node->remove_node(*iter_ndoe);
    }

    char* pvalue = preal_node->document()->allocate_string(value.c_str());
    xml_node* pvalue_node = NULL;
    if (cdata)
    {
        pvalue_node = preal_node->document()->allocate_node(rapidxml::node_cdata, NULL, pvalue);
    }
    else
    {
        pvalue_node = preal_node->document()->allocate_node(rapidxml::node_data, NULL, pvalue);
    }
    preal_node->append_node(pvalue_node);

    return true;
}

bool xml::set_node_attr(xml_node_ptr pnode, const std::string& attr_name, const std::string& attr_value)
{
    assert(pnode);
    assert(!attr_name.empty());

    xml_node* preal_node = xml_node_cast(pnode);
    xml_attr* pattr = preal_node->first_attribute(attr_name.c_str(), 0, false);
    if (pattr)
    {
        preal_node->remove_attribute(pattr);
    }

    char* pname = preal_node->document()->allocate_string(attr_name.c_str());
    char* pvalue = preal_node->document()->allocate_string(attr_value.c_str());
    pattr = preal_node->document()->allocate_attribute(pname, pvalue);
    preal_node->append_attribute(pattr);

    return true;
}

bool xml::remove_node_attr(xml_node_ptr pnode, const std::string& attr_name)
{
    assert(pnode);
    assert(!attr_name.empty());

    xml_node* preal_node = xml_node_cast(pnode);
    xml_attr* pattr = preal_node->first_attribute(attr_name.c_str(), 0, false);
    if (pattr)
    {
        preal_node->remove_attribute(pattr);
    }

    return true;
}


