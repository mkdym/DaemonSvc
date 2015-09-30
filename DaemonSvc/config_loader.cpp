#include "self_path.h"
#include "str_encode.h"
#include "any_lexical_cast.h"
#include "logger.h"
#include "config_loader.h"


using namespace xml;



static void LogXmlNodeErr(const std::string& node_path)
{
    ErrorLog("can not get node[%s]", node_path.c_str());
}

static void LogXmlAttrErr(const std::string& attr_name)
{
    ErrorLog("can not get node attr[%s]", attr_name.c_str());
}



CConfigLoader::CConfigLoader(const tstring& file_path)
{
    load(file_path);
}

CConfigLoader::~CConfigLoader(void)
{
}

void CConfigLoader::load(const tstring& file_path)
{
    std::string config_file = tstr2ansistr(file_path);
    if (config_file.empty())
    {
        config_file = CSelfPath::get_instance_ref().get_dir() + "\\tasks.xml";
    }
    InfoLog("config file: %s", config_file.c_str());

    xml_doc_ptr pdoc = load_xml_file(config_file);
    if (!pdoc)
    {
        ErrorLog("load xml fail");
    }
    else
    {
        parse_all_infos(pdoc, m_ti_infos);
        parse_all_infos(pdoc, m_tp_infos);
        parse_all_infos(pdoc, m_pne_infos);
    }
}

bool CConfigLoader::parse_common_info(const xml_node_ptr pnode, CommonInfo& ci)
{
    bool ret = false;

    do 
    {
        std::string s;
        std::string attr_name = "run_as_logon_users";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        ci.run_as = cast_run_as_type_from_string(s);

        s.clear();
        attr_name = "show_window";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        ci.show_window = any_lexical_cast(s, true);

        ci.cmd = ansistr2tstr(get_node_value(pnode));

        ret = true;

    } while (false);

    return ret;
}

bool CConfigLoader::parse_one_info(const xml_node_ptr pnode, ti_info& info)
{
    bool ret = false;

    do 
    {
        if (!parse_common_info(pnode, info.common_info))
        {
            break;
        }

        std::string s;
        std::string attr_name = "interval_seconds";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.interval_seconds = any_lexical_cast(s, 0);

        ret = true;

    } while (false);

    return ret;
}

bool CConfigLoader::parse_one_info(const xml_node_ptr pnode, tp_info& info)
{
    bool ret = false;

    do 
    {
        if (!parse_common_info(pnode, info.common_info))
        {
            break;
        }

        std::string s;
        std::string attr_name = "type";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.type = PeriodTime::cast_period_type_from_string(s);

        attr_name = "dayofmonth";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.dayofmonth = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "dayofweek";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.dayofweek = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "hour";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.hour = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "minute";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.minute = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "deviation_minutes";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.pt.deviation_minutes = any_lexical_cast(s, 0);

        if (!info.pt.valid(true))
        {
            ErrorLog("period time[%s] is not valid", info.pt.str());
            break;
        }

        ret = true;

    } while (false);

    return ret;
}

bool CConfigLoader::parse_one_info(const xml_node_ptr pnode, pne_info& info)
{
    bool ret = false;

    do 
    {
        if (!parse_common_info(pnode, info.common_info))
        {
            break;
        }

        std::string s;
        std::string attr_name = "proc_path";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.proc_path = ansistr2tstr(s);

        attr_name = "interval_seconds";
        if (!get_node_attr(pnode, attr_name, s))
        {
            LogXmlAttrErr(attr_name);
            break;
        }
        info.interval_seconds = any_lexical_cast(s, 0);

        ret = true;

    } while (false);

    return ret;
}

void CConfigLoader::parse_all_infos(const xml_doc_ptr pdoc, ti_info_list& infos)
{
    InfoLog("parse time_interval_tasks begin");

    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/time_interval_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        ti_info info;
        if (parse_one_info(*iter_node, info))
        {
            InfoLog("time_interval_task_info: %s", info.str().c_str());
            infos.push_back(info);
        }
    }

    InfoLog("parse time_interval_tasks end");
}

void CConfigLoader::parse_all_infos(const xml_doc_ptr pdoc, tp_info_list& infos)
{
    InfoLog("parse time_point_tasks begin");

    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/time_point_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        tp_info info;
        if (parse_one_info(*iter_node, info))
        {
            InfoLog("time_point_task_info: %s", info.str().c_str());
            infos.push_back(info);
        }
    }

    InfoLog("parse time_point_tasks end");
}

void CConfigLoader::parse_all_infos(const xml_doc_ptr pdoc, pne_info_list& infos)
{
    InfoLog("parse proc_non_exist_tasks begin");

    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/proc_non_exist_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        pne_info info;
        if (parse_one_info(*iter_node, info))
        {
            InfoLog("proc_non_exist_task_info: %s", info.str().c_str());
            infos.push_back(info);
        }
    }

    InfoLog("parse proc_non_exist_tasks end");
}

