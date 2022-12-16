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

    vector<FVectorMaker*> create_vector_makers()
    {
        //CREATE FEATURE MAKERS

        vector<string> languages = {"en", "ru"};// languages of texts being vectorized

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

    int index_directory_by_content(const string& dir, const string& sql_filename)
    {
        sqlite3* db;
        int rc = sqlite3_open(sql_filename.c_str(), &db);
        char* z_err_msg = 0;

        vector<FVectorMaker*> vector_makers = create_vector_makers();

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
        for (auto& maker: vector_makers)
            delete maker;
        return 0;
    }

    map<string, vector<pair<string, FVector>>> read_features(const string& db_filename)
    {
        vector<string> languages = {"en", "ru"};

        sqlite3* db;
        int rc = sqlite3_open(db_filename.c_str(), &db);
        map<string, vector<pair<string,FVector>>> files_features;

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
            vector<pair<string, FVector>> path_fvectors;
            for (int i = 0; i < languages.size(); i++)
            {
                FVector fvector(string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i+1))));
                pair<string, FVector> lang_fvector(languages[i], fvector);
                path_fvectors.push_back(lang_fvector);
            }
            files_features[path] = path_fvectors;
        }

        sqlite3_close(db);
        return files_features;
    }

    vector<string> knn_algorithm(const string& text, const string& path_to_db, map<string, double> weights, size_t k)
    {
        //CREATE FEATURE MAKERS
        vector<FVectorMaker*> vector_makers;
        vector<string> languages = {"en", "ru"};

        time_t start = time(NULL);
        cout << "Creating vector makers...";

        #pragma omp parallel for num_threads(omp_get_thread_num())
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

        //Получить все вектора для запроса
        //Сравнить вектора запроса с векторами в БД
        // --Параллельно--
        //Несколько дистанций

        map<string, FVector> features_of_request;
        #pragma omp parallel for num_threads(omp_get_thread_num())
        for (auto& maker: vector_makers)
        {
            features_of_request[maker->get_lang()] = maker->make_feature_vector(text, 0);
        }
        map<string, vector<pair<string, FVector>>> saved = read_features(path_to_db);

        vector<pair<double, string>> distances;

        for (auto& file: saved)
        {
            double distance = 0;
            if (!isnan(distance))
            {
                pair<double, string> dist_path(distance, file.first);
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