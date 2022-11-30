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

    int callback(void *a_param, int argc, char **argv, char **column)
    {
        for (int i=0; i< argc; i++)
            printf("%s,\t", argv[i]);
        printf("\n");

        return 0;
    }

    void save_entire_directory_to_db(const string& dir, sqlite3* &db, int rc, char* &errmsg)
    {
        
        const char* data = "Callback function called";
        
        for(const auto& file: directory_iterator(dir))
        {
            cout << "INDEXING: " << file.path().string() << endl;
            if(is_directory(file))
            {
                try
                {
                    save_entire_directory_to_db(file.path().string(), db, rc, errmsg);

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

        string create = "CREATE    TABLE   IF NOT EXISTS  FILES(" \
                    "FNAME  TEXT    NOT NULL," \
                    "PATH   TEXT    NOT NULL," \
                    "EXT    TEXT    NOT NULL);";
        rc = sqlite3_exec(db, create.c_str(), callback, 0, &z_err_msg);

        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
            return 1;
        }
        else
            cout << "Table created successfully" << endl;

        sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &z_err_msg);
        sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &z_err_msg);
        sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, &z_err_msg);
        save_entire_directory_to_db(dir, db, rc, z_err_msg);
        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
        }
        else
            cout << "Records created successfully" << endl;
        
        sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, &z_err_msg);
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

        sqlite3_close(db);
        return paths;
    }

    void fill_feature_database(const string& dir, const FVectorMaker& vector_maker, const string& lang,  sqlite3* db, int rc, char* errmsg)
    {
        const char* data = "Callback function called";

        for(const auto& file: directory_iterator(dir))
        {
            if(is_directory(file))
            {
                try
                {
                    fill_feature_database(file.path().string(), vector_maker, vector_maker.get_lang(), db, rc, errmsg);
                }
                catch(const filesystem_error& e)
                {
                }
                
            }
            else
            {

                if ((regex_match(file.path().string(), txt_file) || regex_match(file.path().string(), doc_file)) && (file_size(file.path().string()) > 0))
                {
                    cout << "INDEXING: " << file.path().string() << endl;
                    const string serialized_fvector = vector_maker.make_feature_vector(file.path().string(), 1).serialize();
                    string send = "INSERT INTO FEATURES_"  + lang +  " VALUES ('"  + file.path().string() + "', '" + serialized_fvector + "');"; //
                    rc = sqlite3_exec(db, send.c_str(), callback, (void*)data, &errmsg);
                }
            }
        }
    }

    int index_directory_by_content(const string& dir, const string& sql_filename, const FVectorMaker& vector_maker)
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

        string sql = "CREATE TABLE IF NOT EXISTS FEATURES_" + vector_maker.get_lang() + " (PATH TEXT, FVECTOR_JSON TEXT)";
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &z_err_msg);

        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
            return 1;
        }
        else
            cout << "Table created successfully" << endl;

        sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &z_err_msg);
        sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &z_err_msg);
        sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, &z_err_msg);
        fill_feature_database(dir, vector_maker, vector_maker.get_lang(), db, rc, z_err_msg);

        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
        }
        else
            cout << "Records created successfully" << endl;
        
        sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, &z_err_msg);
        sqlite3_close(db);
        return 0;
    }

    map<string, FVector> read_features_from_db(const string& db_filename, const string lang)
    {
        sqlite3* db;
        int rc = sqlite3_open(db_filename.c_str(), &db);
        map<string, FVector> files_features;

        if (rc)
        {
            cout << "Can't open the database" << db_filename << endl;
            return files_features;
        }
        else
        {
            cout << "Successfully opened database" << endl;
        }

        string sql_request = "SELECT * FROM FEATURES_" + lang + ";";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql_request.c_str(), sql_request.length(), &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            cout << "Can not process the request" << endl;
            cout << sqlite3_errstr(sqlite3_extended_errcode(db)) << "\n" << sqlite3_errmsg(db) << endl;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            string path = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            FVector fvector(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))));
            files_features[path] = fvector;
        }

        sqlite3_close(db);
        return files_features;
    }

    vector<string> knn_algorithm(const string& text, const string& path_to_db, const FVectorMaker& maker, size_t k)
    {
        FVector given_fvector = maker.make_feature_vector(text, 0);
        map<string, FVector> set_of_vectors = read_features_from_db(path_to_db, maker.get_lang());

        vector<pair<double, string>> distances;

        for (auto& it: set_of_vectors)
        {
            double distance = correlation(it.second, given_fvector);
            if (!isnan(distance))
            {
                pair<double, string> dist_path(distance, it.first);
                distances.push_back(dist_path);
            }
        }

        sort(distances.begin(), distances.end());

        vector<string> found;

        for (int i = distances.size() - 1; (i >= distances.size() - k) && (i >= 0); i--)
        {
            found.push_back(distances[i].second);
        }
        return found;
    }
}