#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <ctemplate/template.h>

// #include "oj_model.hpp"
#include "oj_model2.hpp"
#include "contest_utils.hpp"

namespace ns_view
{
    using namespace ns_model;

    const std::string template_path = "./template_html/";

    class View
    {
    public:
        View(){}
        ~View(){}
    public:
        void AllExpandHtml(const vector<struct Question> &questions, std::string *html, const User *u = nullptr)
        {
            // 题目的编号 题目的标题 题目的难度
            // 推荐使用表格显示
            // 1. 形成路径
            std::string src_html = template_path + "all_questions.html";
            // 2. 形成数字典
            ctemplate::TemplateDictionary root("all_questions");
            
            if (u && !u->username.empty()) {
                root.ShowSection("user_logged_in");
                root.SetValue("username", u->username);
            } else {
                root.ShowSection("user_not_logged_in");
            }

            for (const auto& q : questions)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("star", q.star);
            }

            //3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            //4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
        void OneExpandHtml(const struct Question &q, std::string *html, const User *u = nullptr)
        {
            // 1. 形成路径
            std::string src_html = template_path + "one_question.html";

            // 2. 形成数字典
            ctemplate::TemplateDictionary root("one_question");
            
            if (u && !u->username.empty()) {
                root.ShowSection("user_logged_in");
                root.SetValue("username", u->username);
            } else {
                root.ShowSection("user_not_logged_in");
            }

            root.SetValue("number", q.number);
            root.SetValue("title", q.title);
            root.SetValue("star", q.star);
            root.SetValue("desc", q.desc);
            
            // 手动进行HTML转义，防止#include <iostream>被浏览器解析为标签
            std::string escaped_header = q.header;
            // 简单的HTML转义
            std::string buffer;
            buffer.reserve(escaped_header.size());
            for(char c : escaped_header) {
                switch(c) {
                    case '&':  buffer.append("&amp;");       break;
                    case '\"': buffer.append("&quot;");      break;
                    case '\'': buffer.append("&apos;");      break;
                    case '<':  buffer.append("&lt;");        break;
                    case '>':  buffer.append("&gt;");        break;
                    default:   buffer.push_back(c);          break;
                }
            }
            root.SetValue("pre_code", buffer);

            //3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
           
            //4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }

        void ProfileExpandHtml(const User &u, const std::unordered_map<std::string, int> &stats, std::string *html)
        {
            std::string src_html = template_path + "profile.html";
            ctemplate::TemplateDictionary root("profile");
            
            root.SetValue("user_id", u.id);
            root.SetValue("username", u.username);
            root.SetValue("email", u.email);
            root.SetValue("nickname", u.nickname.empty() ? u.username : u.nickname);
            root.SetValue("phone", u.phone);
            root.SetValue("created_at", u.created_at);
            
            // Stats
            int total_solved = 0;
            for(auto &kv : stats) {
                total_solved += kv.second;
                if(kv.first == "简单") root.SetValue("easy_solved", std::to_string(kv.second));
                else if(kv.first == "中等") root.SetValue("medium_solved", std::to_string(kv.second));
                else if(kv.first == "困难") root.SetValue("hard_solved", std::to_string(kv.second));
            }
            root.SetValue("total_solved", std::to_string(total_solved));
            
            // Set defaults if 0
            if(stats.find("简单") == stats.end()) root.SetValue("easy_solved", "0");
            if(stats.find("中等") == stats.end()) root.SetValue("medium_solved", "0");
            if(stats.find("困难") == stats.end()) root.SetValue("hard_solved", "0");
            
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &root);
        }

        void DiscussionHtml(std::string *html, const User *u = nullptr)
        {
            std::string src_html = template_path + "discussion.html";
            ctemplate::TemplateDictionary root("discussion");
            
            if (u && !u->username.empty()) {
                root.ShowSection("user_logged_in");
                root.SetValue("username", u->username);
            } else {
                root.ShowSection("user_not_logged_in");
            }

            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &root);
        }

        void ContestHtml(const std::string &json_data, std::string *html, const User *u = nullptr)
        {
            std::string src_html = template_path + "contest.html";
            ctemplate::TemplateDictionary root("contest");

            if (u && !u->username.empty()) {
                root.ShowSection("user_logged_in");
                root.SetValue("username", u->username);
            } else {
                root.ShowSection("user_not_logged_in");
            }
            
            // Parse JSON data
            Json::Reader reader;
            Json::Value root_val;
            if(reader.parse(json_data, root_val)) {
                const Json::Value contests_json = root_val["contests"];
                
                std::vector<ContestEntry> entries;
                time_t now = time(nullptr);
                
                for (unsigned int i = 0; i < contests_json.size(); ++i) {
                    ContestEntry entry;
                    entry.name = contests_json[i]["name"].asString();
                    std::string raw_start = contests_json[i]["start_time"].asString();
                    entry.link = contests_json[i]["link"].asString();
                    
                    // Parse start time "Feb/07/2026 17:35"
                    struct tm tm = {0};
                    if (strptime(raw_start.c_str(), "%b/%d/%Y %H:%M", &tm)) {
                        entry.start_time = mktime(&tm);
                    } else {
                        entry.start_time = 0; 
                    }
                    
                    // Duration: 2 hours (default)
                    entry.end_time = entry.start_time + 7200;
                    
                    // Format times to YYYY-MM-DD HH:mm
                    char buf[64];
                    if (entry.start_time != 0) {
                        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm);
                        entry.start_time_str = buf;
                        
                        struct tm *end_tm = localtime(&entry.end_time);
                        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", end_tm);
                        entry.end_time_str = buf;
                    } else {
                        entry.start_time_str = raw_start;
                        entry.end_time_str = "-";
                    }

                    // Status
                    if (entry.start_time == 0) {
                        entry.status = "未知";
                        entry.status_class = "status-ended";
                    } else if (now < entry.start_time) {
                        entry.status = "未开始";
                        entry.status_class = "status-not-started";
                    } else if (now >= entry.start_time && now <= entry.end_time) {
                        entry.status = "进行中";
                        entry.status_class = "status-running";
                    } else {
                        entry.status = "已结束";
                        entry.status_class = "status-ended";
                    }
                    
                    entries.push_back(entry);
                }
                
                // Sort using utility
                SortContests(entries);
                
                for (const auto& entry : entries) {
                    ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("contests");
                    sub->SetValue("name", entry.name);
                    sub->SetValue("start_time", entry.start_time_str);
                    sub->SetValue("end_time", entry.end_time_str);
                    sub->SetValue("link", entry.link);
                    sub->SetValue("status", entry.status);
                    sub->SetValue("status_class", entry.status_class);
                }
                
                time_t t = root_val["updated_at"].asUInt64();
                char buf[64];
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
                root.SetValue("updated_at", buf);
            }

            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            tpl->Expand(html, &root);
        }
    };
}
