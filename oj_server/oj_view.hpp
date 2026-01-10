#pragma once

#include <iostream>
#include <string>
#include <ctemplate/template.h>

// #include "oj_model.hpp"
#include "oj_model2.hpp"

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
    };
}
