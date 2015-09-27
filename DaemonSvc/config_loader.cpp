#include "self_path.h"
#include "str_encode.h"
#include "any_lexical_cast.h"
#include "logger.h"
#include "config_loader.h"


using namespace xml;



static void LogNoAttrErrA(const std::string& node_path, const std::string& attr_name)
{
    ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
}


CConfigLoader::CConfigLoader(const tstring& file_path)
{
    load(file_path);
}

CConfigLoader::~CConfigLoader(void)
{
}

void CConfigLoader::get(time_interval_task_info_list& infos) const
{
    infos = m_time_interval_tasks_info;
}

void CConfigLoader::get(time_point_task_info_list& infos) const
{
    infos = m_time_point_tasks_info;
}

void CConfigLoader::get(proc_non_exist_task_info_list& infos) const
{
    infos = m_proc_non_exist_tasks_info;
}

void CConfigLoader::load(const tstring& file_path)
{
    tstring config_file = file_path;
    if (config_file.empty())
    {
        config_file = CSelfPath::GetInstanceRef().get_dir() + TSTR("\\tasks.xml");
    }
    InfoLog(TSTR("config file: %s"), config_file.c_str());

    xml_doc_ptr pdoc = load_xml_file(tstr2ansistr(config_file));
    if (!pdoc)
    {
        ErrorLogA("load xml fail");
    }
    else
    {
        load_time_interval_tasks_info(pdoc);
        load_time_point_tasks_info(pdoc);
        load_proc_non_exist_tasks_info(pdoc);
    }
}

void CConfigLoader::load_time_interval_tasks_info(xml_doc_ptr pdoc)
{
    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/time_interval_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        TimeIntervalTaskInfo info;
        std::string attr_name;
        std::string s;

        attr_name = "interval_seconds";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.interval_seconds = any_lexical_cast(s, 0);

        attr_name = "run_as_logon_users";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.run_as = cast_run_as_from_string(s);

        attr_name = "show_window";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.show_window = any_lexical_cast(s, true);

        info.cmd = ansistr2tstr(get_node_value(*iter_node));

        InfoLogA("time_interval_task_info: %s", info.str().c_str());
        m_time_interval_tasks_info.push_back(info);
    }
}

void CConfigLoader::load_time_point_tasks_info(xml_doc_ptr pdoc)
{
    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/time_point_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        TimePointTaskInfo info;
        std::string attr_name;
        std::string s;

        attr_name = "type";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.type = PeriodTime::cast_period_type_from_string(s);

        attr_name = "dayofmonth";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.dayofmonth = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "dayofweek";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.dayofweek = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "hour";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.hour = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "minute";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.minute = any_lexical_cast<unsigned short>(s, 0);

        attr_name = "deviation_minutes";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.pt.deviation_minutes = any_lexical_cast(s, 0);

        if (!info.pt.valid(true))
        {
            ErrorLogA("period time[%s] is not valid", info.pt.str());
            continue;
        }

        attr_name = "run_as_logon_users";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.run_as = cast_run_as_from_string(s);

        attr_name = "show_window";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.show_window = any_lexical_cast(s, true);

        info.cmd = ansistr2tstr(get_node_value(*iter_node));

        InfoLogA("time_point_task_info: %s", info.str().c_str());
        m_time_point_tasks_info.push_back(info);
    }
}

void CConfigLoader::load_proc_non_exist_tasks_info(xml_doc_ptr pdoc)
{
    std::vector<xml_node_ptr> nodes;
    std::string node_path = "root/tasks/proc_non_exist_tasks/task";
    get_node_list(pdoc, NULL, node_path, nodes);
    for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
        iter_node != nodes.end();
        ++iter_node)
    {
        ProcNonExistTaskInfo info;
        std::string attr_name;
        std::string s;

        attr_name = "proc_path";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.proc_path = ansistr2tstr(s);

        attr_name = "interval_seconds";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.interval_seconds = any_lexical_cast(s, 0);

        attr_name = "run_as_logon_users";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.run_as = cast_run_as_from_string(s);

        attr_name = "show_window";
        if (!get_node_attr(*iter_node, attr_name, s))
        {
            LogNoAttrErrA(node_path, attr_name);
            continue;
        }
        info.show_window = any_lexical_cast(s, true);

        info.cmd = ansistr2tstr(get_node_value(*iter_node));

        InfoLogA("proc_non_exist_task_info: %s", info.str().c_str());
        m_proc_non_exist_tasks_info.push_back(info);
    }
}

