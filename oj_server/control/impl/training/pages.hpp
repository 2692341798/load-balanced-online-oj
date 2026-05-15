#pragma once

        bool TrainingListPage(const Request &req, std::string *html)
        {
            User user;
            AuthCheck(req, &user);
            
            view_.TrainingListHtml(html, &user);
            return true;
        }

        bool TrainingDetail(const std::string &id, const Request &req, std::string *html)
        {
            User user;
            AuthCheck(req, &user);
            
            TrainingList list;
            if (!model_.GetTrainingList(id, &list)) {
                *html = "Training List Not Found";
                return false;
            }

            if (list.visibility == "private") {
                if (user.id != list.author_id) {
                    *html = "Access Denied (Private List)";
                    return false;
                }
            }

            std::vector<TrainingListItem> items;
            model_.GetTrainingListProblems(id, user.id, &items);

            view_.TrainingDetailHtml(list, items, html, &user);
            return true;
        }

        bool LoginPage(const Request &req, std::string *html)
        {
            view_.LoginHtml(html);
            return true;
        }
