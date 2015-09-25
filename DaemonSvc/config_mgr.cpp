#include "self_path.h"
#include "str_encode.h"
#include "logger.h"
#include "xml.h"
#include "config_mgr.h"


using namespace xml;


CConfigMgr::CConfigMgr(void)
{
}

CConfigMgr::~CConfigMgr(void)
{
}

void CConfigMgr::load(const tstring& file_path)
{
    HANDLE hConfigFile = INVALID_HANDLE_VALUE;

    do 
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
            break;
        }

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
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.interval_seconds = string_lexical_cast<DWORD>(s, 0);

            attr_name = "run_as_logon_users";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.run_as = cast_run_as_from_string(s);

            attr_name = "show_window";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.show_window = string_lexical_cast<bool>(s, true);

            info.cmd = ansistr2tstr(get_node_value(*iter_node));

            m_time_interval_tasks_info.push_back(info);
        }

        nodes.clear();
        node_path = "root/tasks/time_point_tasks/task";
        get_node_list(pdoc, NULL, node_path, nodes);
        for (std::vector<xml_node_ptr>::const_iterator iter_node = nodes.begin();
            iter_node != nodes.end();
            ++iter_node)
        {
            TimePointTaskInfo info;
            std::string attr_name;

            std::string s;

            //type
            //dayofmonth
            //dayofweek
            //hour
            //minute
            //deviation_minutes

            attr_name = "run_as_logon_users";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.run_as = cast_run_as_from_string(s);

            attr_name = "show_window";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.show_window = string_lexical_cast<bool>(s, true);

            info.cmd = ansistr2tstr(get_node_value(*iter_node));

            m_time_point_tasks_info.push_back(info);
        }

        nodes.clear();
        node_path = "root/tasks/proc_non_exist_tasks/task";
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
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.proc_path = ansistr2tstr(s);

            attr_name = "interval_seconds";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.interval_seconds = string_lexical_cast<DWORD>(s, 0);

            attr_name = "run_as_logon_users";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.run_as = cast_run_as_from_string(s);

            attr_name = "show_window";
            if (!get_node_attr(*iter_node, attr_name, s))
            {
                ErrorLogA("can not get[%s:%s] attr", node_path.c_str(), attr_name.c_str());
                continue;
            }
            info.show_window = string_lexical_cast<bool>(s, true);

            info.cmd = ansistr2tstr(get_node_value(*iter_node));

            m_proc_non_exist_tasks_info.push_back(info);
        }

    } while (false);
}

void CConfigMgr::get(time_interval_task_info_list& infos) const
{
    infos = m_time_interval_tasks_info;
}

void CConfigMgr::get(time_point_task_info_list& infos) const
{
    infos = m_time_point_tasks_info;
}

void CConfigMgr::get(proc_non_exist_task_info_list& infos) const
{
    infos = m_proc_non_exist_tasks_info;
}
