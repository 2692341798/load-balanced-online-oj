#pragma once

#include <string>

namespace ns_model
{
    struct Question
    {
        std::string number;
        std::string title;
        std::string star;
        std::string desc;
        std::string header;
        std::string tail;
        int cpu_limit;
        int mem_limit;
        std::string language_type;
        int status;
    };

    struct User
    {
        std::string id;
        std::string username;
        std::string password;
        std::string email;
        std::string nickname;
        std::string phone;
        std::string avatar;
        std::string created_at;
        int role;
        int status;
    };

    struct InvitationCode
    {
        std::string id;
        std::string code;
        bool is_used;
        std::string used_by;
        std::string created_at;
    };

    struct OperationLog
    {
        std::string id;
        std::string user_id;
        std::string username;
        std::string action;
        std::string target;
        std::string details;
        std::string ip;
        std::string created_at;
    };

    struct Submission
    {
        std::string id;
        std::string user_id;
        std::string question_id;
        std::string question_title;
        std::string result;
        int cpu_time;
        int mem_usage;
        std::string created_at;
        std::string content;
        std::string language;
    };

    struct InlineComment
    {
        std::string id;
        std::string user_id;
        std::string username;
        std::string user_avatar;
        std::string post_id;
        std::string content;
        std::string selected_text;
        std::string parent_id;
        std::string created_at;
    };

    struct Discussion
    {
        std::string id;
        std::string title;
        std::string content;
        std::string author_id;
        std::string author_name;
        std::string author_avatar;
        std::string question_id;
        std::string question_title;
        std::string created_at;
        int likes;
        int views;
        int comments_count;
        bool is_official;
    };

    struct ArticleComment
    {
        std::string id;
        std::string post_id;
        std::string user_id;
        std::string username;
        std::string user_avatar;
        std::string content;
        std::string created_at;
        int likes;
    };

    struct Contest
    {
        std::string id;
        std::string contest_id;
        std::string name;
        std::string start_time;
        std::string end_time;
        std::string link;
        std::string source;
        std::string status;
        std::string last_crawl_time;
    };

    struct TrainingList
    {
        std::string id;
        std::string title;
        std::string description;
        std::string difficulty;
        std::string tags;
        std::string author_id;
        std::string visibility;
        std::string created_at;
        std::string updated_at;
        int likes;
        int collections;
        std::string author_name;
        std::string author_avatar;
        int problem_count;
    };

    struct TrainingListItem
    {
        std::string id;
        std::string training_list_id;
        std::string question_id;
        int order_index;
        std::string question_title;
        std::string question_difficulty;
        std::string user_status;
    };
}

