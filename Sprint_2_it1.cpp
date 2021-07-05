#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */
const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word = "";
        } else {
            word += c;
        }
    }
    words.push_back(word);
    
    return words;
}
    
struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }
    
   
    vector<Document> FindTopDocuments(const string& raw_query) const {
         auto matched_documents = FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
        return matched_documents;
    }
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_in) const {
         auto matched_documents = FindTopDocuments(raw_query, [status_in](int document_id, DocumentStatus status, int rating) {
             return status == status_in;
             });
        return matched_documents;
    }
    
  template <typename PFilter>
  vector<Document> FindTopDocuments(const string& raw_query,const PFilter pfilter) const {
        const Query query = ParseQuery(raw_query);
        vector <Document>  matched_documents;
      for (auto document: FindAllDocuments(query)){
          if (pfilter(document.id,documents_.at(document.id).status,document.rating)){
              matched_documents.push_back(document);
          }
      }
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }


    int GetDocumentCount() const {
        return documents_.size();
    }
    
    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    static int ComputeAverageRating(const vector<int>& ratings) {
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }
    
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
    
    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
               // if (documents_.at(document_id).status == status) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
               // }
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
            });
        }
        return matched_documents;
    }
};
void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}


#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))




template <typename TestFunc>
void RunTestImpl(const TestFunc& test_func, const string& func_name) {
    test_func();
    cerr << func_name << " OK" <<endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func)

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов


void TestFindDocumentFromAddedDocumentContent() {
    SearchServer server;
    const string err_info="Document not added or not finded";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5}); //документ для поиска не пересекается с другими
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, {1, 2, 3, -5});
    server.AddDocument(2, "serega igraet v footbol", DocumentStatus::ACTUAL, {10, 3, -5});
    server.AddDocument(3, "alisa igraet na skripke", DocumentStatus::ACTUAL, {10, 2, 3, 8});
    server.SetStopWords("v na s i"s);
    const auto found_docs0 = server.FindTopDocuments("sasha stroit dom i lubit gulyat s sobakoi"s);
    ASSERT_EQUAL_HINT(found_docs0.size(), 1, err_info); //Запрос = документу, найден только один добавленный документ
    const Document& doc0 = found_docs0[0];
    ASSERT_EQUAL_HINT(doc0.id,0,err_info); //Документ добавлен с правильным id=0
    const auto found_docs1 = server.FindTopDocuments("v nebe letaet samolet"s);
    ASSERT_EQUAL_HINT(found_docs1.size(),0, err_info); //Запрос несоответствующий ни одному документу не найден
}
void TestExcludeStopWordsFromAddedDocumentContent() {
 
    SearchServer server;
    const string err_info="Included stop words";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5}); //добавленные документы не пересекаются
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, {1, 2, 3, -5});
    const auto found_docs0 = server.FindTopDocuments("sasha stroit dom i lubit gulyat s sobakoi"s);
    ASSERT_EQUAL_HINT(found_docs0.size(),1,err_info);  //Без установки стоп-слов находится полностью запрос = документу с правильный id
    const Document& doc0 = found_docs0[0];
    ASSERT_EQUAL_HINT(doc0.id,0,err_info);
    server.SetStopWords("sasha stroit dom i lubit gulyat s sobakoi"s);
    ASSERT_HINT(server.FindTopDocuments("sasha stroit dom i lubit gulyat s sobakoi"s).empty(),err_info); //Нельзя найти полностью добавленный запрос в стоп-слова
}
void TestExcludeMinusWordsFromFind() {
 
    SearchServer server;
    const string err_info="Finded doc included stop words";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5}); //добавленные документы не пересекаются
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, {1, 2, 3, -5});
    const auto found_docs0 = server.FindTopDocuments("sasha stroit dom i lubit gulyat s sobakoi"s);
    ASSERT_EQUAL_HINT(found_docs0.size(),1,err_info); //Запрос = документу без минус слов находит документ с правильным id
    const Document& doc0 = found_docs0[0];
    ASSERT_EQUAL_HINT(doc0.id,0,err_info);
    ASSERT_HINT(server.FindTopDocuments("sasha -stroit dom i lubit gulyat s sobakoi"s).empty(),err_info); //Запрос = документу с минус словом не найден
}
void TestMatchingDocument() {
 
    SearchServer server;
    const string err_info="Matched word not find";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5});
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, {1, 2, 3, -5});
    vector <string> matched_words;
    DocumentStatus status;
    const multiset <string> match_result ={"sasha", "stroit", "dom", "i", "lubit", "gulyat", "s", "sobakoi"};
    tie (matched_words, status) =server.MatchDocument("sasha stroit dom i lubit gulyat s sobakoi"s, 0); //Выбираем полностью документ
    ASSERT_EQUAL_HINT(matched_words.size(),match_result.size(), err_info);
    for (const string& word: matched_words){
        ASSERT_HINT(match_result.count(word),err_info);
    }
    tie (matched_words, status) =server.MatchDocument(""s, 0); //Выбираем пустой запрос
    ASSERT_HINT(matched_words.empty(),err_info);
    tie (matched_words, status) =server.MatchDocument("sasha -stroit dom i lubit gulyat s sobakoi"s, 0); // Выбираем с минус-словами
    ASSERT_HINT(matched_words.empty(),err_info);
}
void TestSortingRelevanceDocument() {
 
    SearchServer server;
    const string err_info="Not sorted";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {1}); //Добавляем документы с одинкаовым рейтингом
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "serega igraet v footbol", DocumentStatus::ACTUAL, {1});
    server.AddDocument(3, "alisa igraet na skripke", DocumentStatus::ACTUAL, {1});
    vector<Document> document = server.FindTopDocuments("stroit dom igraet");
    for (vector <Document>::iterator it = document.begin(); it < document.end(); ++ it){
        ASSERT_HINT(((*it).relevance >= (*++it).relevance),err_info);   //Проверка сортировки
    }
}
void TestAveregeRatiing(){
    SearchServer server;
    const string err_info="Error average rating";
    const vector <int> rating_id0={10, 2, 3, -5};
    const vector <int> rating_id1={0};
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, rating_id0);
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::ACTUAL, rating_id1);
    const auto document0 = server.FindTopDocuments("sasha stroit dom i lubit gulyat s sobakoi");
    const Document& doc0 = document0[0];
    ASSERT_EQUAL_HINT(doc0.rating,2,err_info); // проверяем правильность расчета рейтинга, сравнивая с рассчитаным в ручную
    const auto document1 = server.FindTopDocuments("vasya uchitsya v shkole");
    const Document& doc1 = document1[0];
    ASSERT_EQUAL_HINT(doc1.rating,0,err_info); // проверяем правильность расчета рейтинга, сравнивая с рассчитаным в ручную == 0
}

void TestFilterPredicate(){
    SearchServer server;
    const string err_info="Error - filter not working";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5});
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::IRRELEVANT, {1, 2, 3, -5});
    server.AddDocument(2, "serega igraet v footbol", DocumentStatus::BANNED, {10, 3, -5});
    server.AddDocument(3, "alisa igraet na skripke", DocumentStatus::REMOVED, {10, 2, 3, 8});
    const auto document0 = server.FindTopDocuments("stroit",DocumentStatus::ACTUAL); // Поиск по статусу
    const Document& doc0 = document0[0];
    ASSERT_EQUAL_HINT(doc0.id,0,err_info);
    const auto document1 = server.FindTopDocuments("vasya sasha serega alisa",[](int document_id, DocumentStatus status, int rating) { return document_id == 1; }); //поиск по id
    const Document& doc1 = document1[0];
    ASSERT_EQUAL_HINT(doc1.id,1,err_info);
    const auto document2 = server.FindTopDocuments("vasya sasha serega alisa",[](int document_id, DocumentStatus status, int rating) { return rating > 0; }); //поиск по рейтингу
    const Document& doc2 = document2[0];
    ASSERT_HINT((doc2.rating >0),err_info);
}

void TestFilterStatus(){
    SearchServer server;
    const string err_info="Error - status filter not working";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5});
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::IRRELEVANT, {1, 2, 3, -5});
    server.AddDocument(2, "serega igraet v footbol", DocumentStatus::BANNED, {10, 3, -5});
    server.AddDocument(3, "alisa igraet na skripke", DocumentStatus::REMOVED, {10, 2, 3, 8});
    const auto document0 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::ACTUAL);  // Проверка поиска по статусу
    const Document& doc0 = document0[0];
    ASSERT_EQUAL_HINT(doc0.id,0,err_info);
    const auto document1 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::IRRELEVANT);
    const Document& doc1 = document1[0];
    ASSERT_EQUAL_HINT(doc1.id,1,err_info);
    const auto document2 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::BANNED);
    const Document& doc2 = document2[0];
    ASSERT_EQUAL_HINT(doc2.id,2,err_info);
    const auto document3 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::REMOVED);
    const Document& doc3 = document3[0];
    ASSERT_EQUAL_HINT(doc3.id,3,err_info);
}

void TestRelevance(){
    SearchServer server;
    const string err_info="Error - incorrect relevance";
    server.AddDocument(0, "sasha stroit dom i lubit gulyat s sobakoi", DocumentStatus::ACTUAL, {10, 2, 3, -5});
    server.AddDocument(1, "vasya uchitsya v shkole", DocumentStatus::IRRELEVANT, {1, 2, 3, -5});
    server.AddDocument(2, "serega igraet v footbol", DocumentStatus::BANNED, {10, 3, -5});
    server.AddDocument(3, "alisa igraet na skripke", DocumentStatus::REMOVED, {10, 2, 3, 8});
    const auto document0 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::ACTUAL);  //Проверяем релевантность с посчитанной вручную
    const Document& doc0 = document0[0];
    ASSERT_HINT((abs(doc0.relevance - 0.173287) < 10e-6),err_info);
    const auto document1 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::IRRELEVANT);
    const Document& doc1 = document1[0];
    ASSERT_HINT((abs(doc1.relevance - 0.346574) < 10e-6),err_info);
    const auto document2 = server.FindTopDocuments("vasya sasha serega alisa",DocumentStatus::BANNED);
    const Document& doc2 = document2[0];
    ASSERT_HINT((abs(doc2.relevance - 0.346574) < 10e-6),err_info);
}



void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFindDocumentFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWordsFromFind);
    RUN_TEST(TestMatchingDocument);
    RUN_TEST(TestSortingRelevanceDocument);
    RUN_TEST(TestAveregeRatiing);
    RUN_TEST(TestFilterPredicate);
    RUN_TEST(TestFilterStatus);
    RUN_TEST(TestRelevance);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
