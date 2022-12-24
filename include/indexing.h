#include "boost/filesystem.hpp"
#include <omp.h>
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

    vector<FVectorMaker*> create_vector_makers(const vector<string> languages)
    {
        //CREATE FEATURE MAKERS

        vector<FVectorMaker*> vector_makers;
        vector_makers.reserve(languages.size());

        time_t start = time(NULL);
        cout << "Creating vector makers...";
        int threads = omp_get_thread_num();
        #pragma omp parallel for num_threads(threads)
        for (auto& language: languages)
        {
            string dict = "..\\..\\data\\dictionaries\\" + language + "_dictionary.txt";
            string stopwords = "..\\..\\data\\stopwords\\" + language + "_stopwords.txt";
            FVectorMaker* temp = new FVectorMaker(dict, stopwords, language);
            vector_makers.push_back(temp);
        }
        
        time_t end = time(NULL) - start;
        cout << "Created vector makers " << end << "sec" << endl;
        //

        return vector_makers;
    }

    void create_vector_makers_on_heap(const vector<string> languages, vector<FVectorMaker*>* &pointer_to_vector_makers)
    {
        //CREATE FEATURE MAKERS ALLOCATED ON HEAP
        vector<FVectorMaker*>* vector_makers = pointer_to_vector_makers;
        vector_makers->reserve(languages.size());

        time_t start = time(NULL);
        cout << "Creating vector makers...";
        int threads = omp_get_thread_num();
        #pragma omp parallel for num_threads(threads)
        for (auto& language: languages)
        {
            string dict = "..\\..\\data\\dictionaries\\" + language + "_dictionary.txt";
            string stopwords = "..\\..\\data\\stopwords\\" + language + "_stopwords.txt";
            FVectorMaker* temp = new FVectorMaker(dict, stopwords, language);
            vector_makers->push_back(temp);
        }
        
        time_t end = time(NULL) - start;
        cout << "Created vector makers " << end << "sec" << endl;
        pointer_to_vector_makers = vector_makers;
        //
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

    vector<string> find_in_db(const string& request, const string& sql_filename)
    {
        sqlite3* db;
        int rc = sqlite3_open(sql_filename.c_str(), &db);
        vector<string> paths;

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
            paths.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        }

        sqlite3_close(db);
        return paths;
    }

    vector<string> find_in_db(const string& request, const string& sql_filename, vector<string>& found)
    {
        sqlite3* db;
        int rc = sqlite3_open(sql_filename.c_str(), &db);

        found.clear();

        if (rc)
        {
            cout << "Can't open the database" << sql_filename << endl;
            return found;
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
            found.push_back(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
        }

        sqlite3_close(db);
        return found;
    }

    void fill_feature_database(const string& dir, const vector<FVectorMaker*>& vector_makers, sqlite3* db, int rc, char* errmsg)
    {
        const char* data = "Callback function called";

        for(const auto& file: directory_iterator(dir))
        {
            if(is_directory(file))
            {
                try
                {
                    fill_feature_database(file.path().string(), vector_makers, db, rc, errmsg);
                }
                catch(const filesystem_error& e)
                {
                }
                
            }
            else
            {

                if ((regex_match(file.path().string(), txt_file) || regex_match(file.path().string(), doc_file)) && (file_size(file.path().string()) > 0))
                {
                    map<string, string> feature_vectors;
                    cout << "INDEXING: " << file.path().string() << endl;

                    #pragma omp parallel
                    {
                        #pragma omp for
                        for (auto& maker: vector_makers)
                        {
                            const string serialized_fvector = maker->make_feature_vector(file.path().string(), 1).serialize();
                            feature_vectors[maker->get_lang()] = serialized_fvector;
                        }
                    }
                    
                    string send = "INSERT INTO FEATURES (PATH,";
                    for (auto& fvector: feature_vectors)
                    {
                        send += " FVECTOR_" + fvector.first + ",";
                    }
                    if (!send.empty())
                        send.resize(send.size() - 1);

                    send += ") VALUES ('" + file.path().string() + "', ";
                    for (auto& fvector: feature_vectors)
                    {
                        send += "'" + fvector.second + "', ";
                    }
                    if (!send.empty())
                        send.resize(send.size() - 2);
                    send += ");";


                    rc = sqlite3_exec(db, send.c_str(), callback, (void*)data, &errmsg);
                }
            }
        }
    }

    int index_directory_by_content(const string& dir, const string& sql_filename, vector<string> languages, vector<FVectorMaker*> vector_makers)
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

        string sql = "CREATE TABLE IF NOT EXISTS FEATURES (PATH TEXT, ";
        for (auto& fvector: vector_makers)
        {
            sql += "FVECTOR_" + fvector->get_lang() + " TEXT,";
        }

        if (!sql.empty())
            sql.resize(sql.size() - 1);

        sql += ");";

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

        fill_feature_database(dir, vector_makers, db, rc, z_err_msg);

        if (rc != SQLITE_OK)
        {
            cout << "SQL_ERROR" << z_err_msg << endl;
            sqlite3_free(z_err_msg);
        }
        else
            cout << "Records created successfully" << endl;
        
        sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, &z_err_msg);
        sqlite3_close(db);
        // for (auto& maker: vector_makers)
        //     delete maker;
        return 0;
    }

    map<string, map<string, FVector>> read_features(const string& db_filename)
    {
        vector<string> languages = {"en", "ru"};

        sqlite3* db;
        int rc = sqlite3_open(db_filename.c_str(), &db);
        map<string, map<string, FVector>> files_features;

        if (rc)
        {
            cout << "Can't open the database" << db_filename << endl;
            return files_features;
        }
        else
        {
            cout << "Successfully opened database" << endl;
        }

        string sql_request = "SELECT * FROM FEATURES;";
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
            map<string, FVector> path_fvectors;
            for (int i = 0; i < languages.size(); i++)
            {
                FVector fvector(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i+1))));
                path_fvectors[languages[i]] = fvector;
            }
            files_features[path] = path_fvectors;
        }

        sqlite3_close(db);
        return files_features;
    }

    vector<string> knn_algorithm(const string& text, const string& path_to_db, size_t k, const vector<FVectorMaker*> vector_makers, vector<string>& found)
    {
        map<string, FVector> features_of_request;
        vector<string> languages;
        #pragma omp parallel for num_threads(4)
        for (auto& maker: vector_makers)
        {
            features_of_request[maker->get_lang()] = maker->make_feature_vector(text, 0);
            languages.push_back(maker->get_lang());
        }

        map<string, map<string, FVector>> loaded = read_features(path_to_db);

        vector<pair<double, string>> distances;

        for (auto& it : features_of_request)
            cout << it.first << ": " << it.second.serialize() << endl;

        for (auto& file: loaded)
        {
            cout << file.first << endl;
            double summarized_distance = 0;
            for (auto& lang : file.second)
            {    
                //cout << lang.first << " : " << intersection(features_of_request[lang.first], loaded[file.first][lang.first]) << endl;
                summarized_distance += intersection(features_of_request[lang.first], loaded[file.first][lang.first]);
            }
            pair<double, string> path_distance(summarized_distance, file.first);
            distances.push_back(path_distance);
        }
        sort(distances.begin(), distances.end());

        found.clear();
        for (int i = distances.size() - 1; (i >= (int)(distances.size()) - (int)(k)) && (i >= 0); i--)
        {
            found.push_back(distances[i].second);
            cout << distances[i].second << " : " << distances[i].first << endl;
        }
        return found;
    }
}