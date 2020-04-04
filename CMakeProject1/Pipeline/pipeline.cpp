#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;


struct Email {
    string from;
    string to;
    string body;
};


class Worker {
public:
    virtual ~Worker() = default;
    virtual void Process(unique_ptr<Email> email) = 0;
    virtual void Run() {
        // только первому worker-у в пайплайне нужно это имплементировать
        throw logic_error("Unimplemented");
    }

protected:
    // реализации должны вызывать PassOn, чтобы передать объект дальше
    // по цепочке обработчиков
    void PassOn(unique_ptr<Email> email) const {
        if (!nxt) {
            return;
        }
        nxt->Process(move(email));
    }

public:
    void SetNext(unique_ptr<Worker> next) {
        nxt = move(next);
    }

private:
    unique_ptr<Worker> nxt = nullptr;
};


class Reader : public Worker {
public:
    Reader(istream& is)
        : is(is)
    {}

    virtual void Process(unique_ptr<Email> email) override {
    }

    virtual void Run() override {
        Email tmp;
        while (getline(is, tmp.from)) {
            getline(is, tmp.to);
            getline(is, tmp.body);
            auto ptr = make_unique<Email>(tmp);
            PassOn(move(ptr));
        }
    }

private:
    istream& is;
};


class Filter : public Worker {
public:
    using Function = function<bool(const Email&)>;

public:
    Filter(Function func)
        : func(func)
    {}

    virtual void Process(unique_ptr<Email> email) override {
        if (func(*email)) {
            PassOn(move(email));
        }
    }

private:
    Function func;
};


class Copier : public Worker {
public:
    Copier(const string& to)
        : to(to)
    {}

    virtual void Process(unique_ptr<Email> email) override {
        if (to != email->to) {
            Email copy_email{ email->from, to, email->body };
            PassOn(move(email));
            PassOn(move(make_unique<Email>(copy_email)));
        }
        else {
            PassOn(move(email));
        }
    }

private:
    string to;
};


class Sender : public Worker {
public:
    Sender(ostream& os)
        : os(os)
    {}

    virtual void Process(unique_ptr<Email> email) override {
        os << email->from << "\n" << email->to << "\n" << email->body << "\n";
        PassOn(move(email));
    }

private:
    ostream& os;
};


// реализуйте класс
class PipelineBuilder {
public:
    // добавляет в качестве первого обработчика Reader
    explicit PipelineBuilder(istream& in) {
        workers.push_back(make_unique<Reader>(in));
    };

    // добавляет новый обработчик Filter
    PipelineBuilder& FilterBy(Filter::Function filter) {
        workers.push_back(make_unique<Filter>(filter));
        return *this;
    }

    // добавляет новый обработчик Copier
    PipelineBuilder& CopyTo(string recipient) {
        workers.push_back(make_unique<Copier>(recipient));
        return *this;
    }

    // добавляет новый обработчик Sender
    PipelineBuilder& Send(ostream& out) {
        workers.push_back(make_unique<Sender>(out));
        return *this;
    }

    // возвращает готовую цепочку обработчиков
    unique_ptr<Worker> Build() {
        unique_ptr<Worker> root = move(workers.back());
        workers.pop_back();
        while (!workers.empty()) {
            workers.back()->SetNext(move(root));
            root = move(workers.back());
            workers.pop_back();
        }
        return root;
    }

private:
    vector<unique_ptr<Worker>> workers;
};


void TestSanity() {
    string input = (
        "erich@example.com\n"
        "richard@example.com\n"
        "Hello there\n"

        "erich@example.com\n"
        "ralph@example.com\n"
        "Are you sure you pressed the right button?\n"

        "ralph@example.com\n"
        "erich@example.com\n"
        "I do not make mistakes of that kind\n"
        );
    istringstream inStream(input);
    ostringstream outStream;

    PipelineBuilder builder(inStream);
    builder.FilterBy([](const Email& email) {
        return email.from == "erich@example.com";
        });
    builder.CopyTo("richard@example.com");
    builder.Send(outStream);
    auto pipeline = builder.Build();

    pipeline->Run();

    string expectedOutput = (
        "erich@example.com\n"
        "richard@example.com\n"
        "Hello there\n"

        "erich@example.com\n"
        "ralph@example.com\n"
        "Are you sure you pressed the right button?\n"

        "erich@example.com\n"
        "richard@example.com\n"
        "Are you sure you pressed the right button?\n"
        );

    ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSanity);
    return 0;
}
