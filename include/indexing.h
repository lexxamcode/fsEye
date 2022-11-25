#include "boost/json.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/filesystem.hpp"

#include "../source/headers/FVectorMaker.h"

#include <sqlite3.h>
namespace indexing
{
    using namespace std;
    using namespace feature_vector;
    namespace pt = boost::property_tree;
    using namespace boost::filesystem;

    const std::regex json_file("(.*?)(\.json)");

    void save_by_content_entire_directory_to_json(const string& dir, const FVectorMaker& vector_maker, boost::json::object& main_json_target)
    {
        
        for (const auto& file: boost::filesystem::directory_iterator(dir))
        {
            if (boost::filesystem::is_directory(file))
                save_by_content_entire_directory_to_json(file.path().string(), vector_maker, main_json_target);
        
            else
            {
                boost::json::object current_file_fvector;
                FVector features = vector_maker.make_feature_vector(file.path().string(), 1);
                for (auto& it: features.get_sparse_vector())
                {
                    current_file_fvector[to_string(it.first)] = it.second;
                }
                main_json_target[file.path().string()] = current_file_fvector;
                cout << file.path().string() << endl;
            }
        }
    }

    void save_directory_to_json(const string& dir, boost::json::object& json_target)
    {
       for(const auto& file: directory_iterator(dir))
       {
            if(is_directory(file))
            {
                try
                {
                    save_directory_to_json(file.path().string(), json_target);
                }
                catch(const filesystem_error& e)
                {
                }
                
            }
            else
            {
                json_target[file.path().filename().string()] = file.path().string(); 
            }
       }
    }

    int callback(void *a_param, int argc, char **argv, char **column)
    {
        for (int i=0; i< argc; i++)
            printf("%s,\t", argv[i]);
        printf("\n");

        return 0;
    }

    void save_entire_directory_to_sql(const string& dir, sqlite3* &db, int rc, char* &errmsg)
    {
        
        const char* data = "Callback function called";
        // sqlite3* db;
        // int rc = sqlite3_open(sql_filename.c_str(), &db);
        for(const auto& file: directory_iterator(dir))
       {
            cout << "INDEXING: " << file.path().string() << endl;
            if(is_directory(file))
            {
                try
                {
                    save_entire_directory_to_sql(file.path().string(), db, rc, errmsg);

                }
                catch(const filesystem_error& e)
                {
                }
                
            }
            else
            {
                string fname = file.path().filename().string();
                string path = file.path().string();
                string ext = file.path().extension().string();

                string send = "INSERT INTO FILES VALUES " + string("('") + fname + string("','") + path + string("','") + ext + string("');");
                cout << send << endl;
                rc = sqlite3_exec(db, send.c_str(), callback, (void*)data, &errmsg);
            }
       }
    }

    int index_directory_to_db(const string& dir, const string& sql_filename)
    {
        sqlite3* db;
        int rc = sqlite3_open(sql_filename.c_str(), &db);
        char* z_err_msg = 0;

        if (rc)
        {
            cout << "Can't open the database" << sql_filename << endl;
            return 1;
        }
        else
        {
            cout << "Successfully opened database" << endl;
        }

        string sql = "CREATE    TABLE   FILES(" \
                    "FNAME  TEXT    NOT NULL," \
                    "PATH   TEXT    NOT NULL," \
                    "EXT    TEXT    NOT NULL);";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &z_err_msg);

        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
            return 1;
        }
        else
            cout << "Table created successfully" << endl;


        save_entire_directory_to_sql(dir, db, rc, z_err_msg);
        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
        }
        else
            cout << "Records created successfully" << endl;
        
        sqlite3_close(db);
        return 0;
    }

    unordered_set<string> find_in_db(const string& request, const string& sql_filename)
    {
        sqlite3* db;
        int rc = sqlite3_open(sql_filename.c_str(), &db);
        unordered_set<string> paths;

        if (rc)
        {
            cout << "Can't open the database" << sql_filename << endl;
            return paths;
        }
        else
        {
            cout << "Successfully opened database" << endl;
        }

        string sql_request = "SELECT PATH FROM FILES WHERE FNAME='" + request + "'";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql_request.c_str(), sql_request.length(), &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            cout << "Can not process the request" << endl;
            cout << sqlite3_errstr(sqlite3_extended_errcode(db)) << "\n" << sqlite3_errmsg(db) << endl;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            paths.insert(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        }

        //sqlite3_free(stmt);
        //sqlite3_free(ErrMessage);
        sqlite3_close(db);
        return paths;
    }

    map<string, string> read_filenames_from_json(const string& path_to_json_file)
    {
        if (!std::regex_match(path_to_json_file, json_file))
            throw "Not a .json file";
        
        pt::ptree load_ptree_root;
        map<string, string> filenames_paths;

        try
        {
            pt::read_json(path_to_json_file, load_ptree_root);
        }
        catch(pt::json_parser::json_parser_error e)
        {
            cout << "Error while reading .json file" << endl;
            return filenames_paths;
        }

        for (auto& filename: load_ptree_root)
        {
            filenames_paths[filename.first] = filename.second.data();
        }

        return filenames_paths;

    }

    map<string, FVector> read_features_from_json(const string& path_to_json_file, size_t dict_size)
    {
        if (!std::regex_match(path_to_json_file, json_file))
            throw "Not a .json file";

        map<string, FVector> result_data;

        pt::ptree load_ptree_root;
        try
        {
            pt::read_json(path_to_json_file, load_ptree_root);
        }
        catch (pt::json_parser::json_parser_error)
        {
            cout << "Error while reading .json file" << endl;
            return result_data;
        }

        for (auto& path: load_ptree_root)
        {
            map<size_t, size_t> sparse_vector;
            for (auto& index: path.second)
            {
                auto& value = index.second;
                sparse_vector[stoi(index.first)] = stoi(value.data());
            }
            FVector temp(sparse_vector, dict_size);
            result_data[path.first] = temp;
        }

        return result_data;
    }
    // Now read from index.json

    vector<string> knn_algorithm(const string& text, const string& path_to_json, const FVectorMaker& maker, size_t k)
    {
        FVector given_fvector = maker.make_feature_vector(text, 0);
        map<string, FVector> set_of_vectors = read_features_from_json(path_to_json, maker.get_dict_size());

        vector<pair<double, string>> distances;

        for (auto& it: set_of_vectors)
        {
            double distance = correlation(it.second, given_fvector);
            pair<double, string> dist_path(distance, it.first);
            distances.push_back(dist_path);
        }

        sort(distances.begin(), distances.end());

        vector<string> found;

        for (int i = distances.size() - 1; i >= distances.size() - k; i--)
        {
            found.push_back(distances[i].second);
        }
        return found;
    }

    // bool find_file(const path & dir_path, const std::string & file_name, path& path_found)
    // {
    //     if (!exists( dir_path )) 
    //         return false;

    //     directory_iterator end_itr; // default construction yields past-the-end
    //     for ( directory_iterator itr( dir_path ); itr != end_itr; ++itr)        
    //     {
    //         if (is_directory(itr->status()))
    //         {
    //             cout << itr->status() << endl;
    //             if (find_file(itr->path(), file_name, path_found))
    //                 return true;
    //         }
    //         else if (itr->path().leaf() == file_name) // see below
    //         {
    //             path_found = itr->path();
    //             return true;
    //         }
    //     }
    //     return false;
    // }
}