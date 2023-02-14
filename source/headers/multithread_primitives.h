//
// Created by lexam on 10.02.2023.
//

#include "indexing.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#ifndef FSEYE_MULTITHREAD_PRIMITIVES_H
#define FSEYE_MULTITHREAD_PRIMITIVES_H

#endif //FSEYE_MULTITHREAD_PRIMITIVES_H

namespace multithread_primitives
{
    using namespace std;

    // Events
    const LPCSTR index_event_name = (LPCSTR)"index_event";
    const LPCSTR search_event_name = (LPCSTR)"search_event";

    HANDLE index_event, search_event;
    void create_events()
    {
        index_event = CreateEvent(nullptr, false, false, index_event_name);
        search_event = CreateEvent(nullptr, false, false, search_event_name);
    }

    // Mutexes
    const LPCSTR index_mutex_name = (LPCSTR)"index_mutex";
    const LPCSTR search_mutex_name = (LPCSTR)"search_mutex";
    const LPCSTR load_mutex_name = (LPCSTR)"load_mutex";

    HANDLE search_mutex, index_mutex, load_mutex;
    void create_mutexes()
    {
        index_mutex = CreateMutex(nullptr, false, index_mutex_name);
        search_mutex = CreateMutex(nullptr, false, search_mutex_name);
        load_mutex = CreateMutex(nullptr, false, load_mutex_name);
    }


    // Mailslot addresses
    const string index_thread_mailslot_address = "\\\\.\\mailslot\\index_ms";
    const string search_thread_mailslot_address = "\\\\.\\mailslot\\search_ms";


    // The structure is going to be passed to the thread with its creation
    // as an argument
    struct LoadThreadPackage
    {
        vector<FVectorMaker*>* ptr_to_vector_makers;
        string data_folder_path;
        vector<string> languages;
        bool* loaded;

        LoadThreadPackage(): ptr_to_vector_makers(nullptr), data_folder_path(""), loaded(nullptr) { }
    };

    // The structure is going to be passed to the thread
    // using mailslot
    struct IndexThreadPackage
    {
        bool by_content;

        bool* indexing_ptr;
        string directory;
        string database_filename;
        vector<string> languages;
        vector<FVectorMaker*>* feature_makers;

        IndexThreadPackage(): by_content(false), directory(""), database_filename(""), feature_makers(nullptr) { }
        static IndexThreadPackage* create_index_package()
        {
            IndexThreadPackage* package = new IndexThreadPackage();
            return package;
        }
        static void delete_package(IndexThreadPackage* package_ptr)
        {
            delete package_ptr;
        }
    };

    // The structure is going to be passed to the thread
    // using mailslot
    struct SearchThreadPackage
    {
        bool by_content;

        bool* searching_ptr;
        string database_filename;
        string request;
        vector<FVectorMaker*>* feature_makers;
        int k;

        vector<string>* found;

        SearchThreadPackage(): by_content(false), searching_ptr(nullptr), database_filename(""), request(""),
                               feature_makers(nullptr), k(0), found(nullptr) { }
        static SearchThreadPackage* create_package()
        {
            SearchThreadPackage* package = new SearchThreadPackage();
            return package;
        }
        static void delete_package(SearchThreadPackage* package_ptr)
        {
            delete package_ptr;
        }
    };

    // Loading vector makers in separate thread
    // It does not need to receive any information with mailslot
    // because it starts only with start of main process,
    // and it gets its arguments in lpParam
    DWORD WINAPI loading_thread_function(LPVOID lpParam)
    {
        // bool loaded is a critical section of window class
        HANDLE load_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, load_mutex_name);

        LoadThreadPackage* info = (LoadThreadPackage*)lpParam;

        WaitForSingleObject(load_mutex, INFINITE);
        *(info->loaded) = false;
        ReleaseMutex(load_mutex);

        for (auto& it: info->languages)
            cout << it << endl;
        indexing::create_vector_makers_on_heap(info->data_folder_path, info->languages, info->ptr_to_vector_makers);
        cout << "LOADED" << endl;

        WaitForSingleObject(load_mutex, INFINITE);
        *(info->loaded) = true;
        ReleaseMutex(load_mutex);

        ExitThread(0);
    }

    DWORD WINAPI indexing_thread_function(LPVOID lpParam)
    {
        HANDLE index_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, index_event_name);
        HANDLE index_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, index_mutex_name);
        HANDLE mailslot = CreateMailslot(index_thread_mailslot_address.c_str(), sizeof(IndexThreadPackage*),
                                         MAILSLOT_WAIT_FOREVER, nullptr);
        IndexThreadPackage* info = nullptr;

        while(true)
        {
            WaitForSingleObject(index_event, INFINITE);
            // When event occured, Thread reads data package and performs its functional

            ReadFile(mailslot, &info, sizeof(IndexThreadPackage*), nullptr, nullptr);

            WaitForSingleObject(index_mutex, INFINITE);
            *(info->indexing_ptr) = true;
            ReleaseMutex(index_mutex);

            if (info->by_content)
                indexing::index_directory_by_content(info->directory, info->database_filename, info->languages, *(info->feature_makers));
            else
                indexing::index_directory_to_db(info->directory, info->database_filename);

            WaitForSingleObject(index_mutex, INFINITE);
            *(info->indexing_ptr) = false;
            ReleaseMutex(index_mutex);
        }
    }

    DWORD WINAPI search_thread_function(LPVOID lpParam)
    {
        HANDLE search_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, search_event_name);
        HANDLE search_mutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, search_mutex_name);
        HANDLE mailslot = CreateMailslot(search_thread_mailslot_address.c_str(), sizeof(SearchThreadPackage*),
                                         MAILSLOT_WAIT_FOREVER, nullptr);

        SearchThreadPackage* info = nullptr;

        while(true)
        {
            WaitForSingleObject(search_event, INFINITE);
            // When event occured, Thread reads data package and performs its functional

            ReadFile(mailslot, &info, sizeof(IndexThreadPackage*),
                     nullptr, nullptr);

            WaitForSingleObject(search_mutex, INFINITE);
            *(info->searching_ptr) = true;
            ReleaseMutex(search_mutex);

            if (info->by_content)
                indexing::knn_algorithm(info->request, info->database_filename,
                                        info->k, *(info->feature_makers), *(info->found));

            WaitForSingleObject(search_mutex, INFINITE);
            *(info->searching_ptr) = false;
            ReleaseMutex(search_mutex);
        }
    }

    HANDLE send_package_to_indexing_thread(IndexThreadPackage* package, bool by_content, bool* indexing_ptr,
                                         const string& directory, const string& database_filename,
                                         const vector<string>& languages, vector<FVectorMaker*>* feature_makers)
    {
        package->by_content = by_content;
        package->indexing_ptr = indexing_ptr;
        package->directory = directory;
        package->database_filename = database_filename;
        package->languages = languages;
        package->feature_makers = feature_makers;

        HANDLE mailslot = CreateFile(index_thread_mailslot_address.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                     nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        WriteFile(mailslot, &package, sizeof(IndexThreadPackage*), nullptr, nullptr);
        SetEvent(index_event);

        return mailslot;
    }

    HANDLE send_package_to_searching_thread(SearchThreadPackage* package, bool by_content, bool* searching_ptr,
                                         const string& database_filename, const string& request,
                                         vector<FVectorMaker*>* feature_makers, int k)
    {
        package->by_content = by_content;
        package->searching_ptr = searching_ptr;
        package->database_filename = database_filename;
        package->request = request;
        package->feature_makers = feature_makers;
        package->k = k;

        HANDLE mailslot = CreateFile(search_thread_mailslot_address.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                     nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        WriteFile(mailslot, &package, sizeof(SearchThreadPackage*), nullptr, nullptr);
        SetEvent(index_event);

        return mailslot;
    }
}
