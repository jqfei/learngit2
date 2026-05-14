#include <bits/stdc++.h>
using namespace std;

/* =========================
   1. Basic Types
========================= */

struct Chunk {
    string data;
};

/* =========================
   2. Logger
========================= */

class Logger {
public:
    void info(const string& msg) {
        cout << "[INFO] " << msg << endl;
    }

    void error(const string& msg) {
        cout << "[ERROR] " << msg << endl;
    }
};

/* =========================
   3. Configs
========================= */

struct ProjectConfig {
    int bandwidthLimit;
    int retryCount;
    bool checksumValidation;
};

struct ItemConfig {
    int retryOverride = -1;
    string priority = "normal";
};

/* =========================
   4. DataSource / Target
========================= */

class DataSource {
public:
    virtual optional<Chunk> nextChunk() = 0;
    virtual ~DataSource() = default;
};

class Target {
public:
    virtual void writeChunk(const Chunk& chunk) = 0;
    virtual bool commit() = 0;
    virtual ~Target() = default;
};

/* =========================
   5. Mock implementations
========================= */

class FileDataSource : public DataSource {
    int cnt = 3;
public:
    optional<Chunk> nextChunk() override {
        if (cnt-- > 0) return Chunk{"file_chunk"};
        return nullopt;
    }
};

class ApplianceTarget : public Target {
public:
    void writeChunk(const Chunk& chunk) override {
        // simulate write
    }

    bool commit() override {
        return true;
    }
};

/* =========================
   6. Replication Item Base
========================= */

class ReplicationItem {
protected:
    string id;
    string type;
    ItemConfig itemConfig;

    unique_ptr<DataSource> source;
    unique_ptr<Target> target;

public:
    ReplicationItem(string id,
                    string type,
                    ItemConfig cfg,
                    unique_ptr<DataSource> s,
                    unique_ptr<Target> t)
        : id(move(id)),
          type(move(type)),
          itemConfig(cfg),
          source(move(s)),
          target(move(t)) {}

    virtual void replicate(const ProjectConfig& projectCfg,
                           Logger& logger) {

        int retry = (itemConfig.retryOverride >= 0)
                    ? itemConfig.retryOverride
                    : projectCfg.retryCount;

        logger.info("Replicating item " + id +
                    " retry=" + to_string(retry));

        while (auto chunk = source->nextChunk()) {
            target->writeChunk(*chunk);
        }

        target->commit();
    }

    string getId() const { return id; }
    string getType() const { return type; }

    virtual ~ReplicationItem() = default;
};

/* =========================
   7. Concrete Items
========================= */

class FileItem : public ReplicationItem {
public:
    FileItem(string id,
             ItemConfig cfg,
             unique_ptr<DataSource> s,
             unique_ptr<Target> t)
        : ReplicationItem(id, "file", cfg, move(s), move(t)) {}
};

class BlockItem : public ReplicationItem {
public:
    BlockItem(string id,
              ItemConfig cfg,
              unique_ptr<DataSource> s,
              unique_ptr<Target> t)
        : ReplicationItem(id, "block", cfg, move(s), move(t)) {}
};

class ObjectItem : public ReplicationItem {
public:
    ObjectItem(string id,
               ItemConfig cfg,
               unique_ptr<DataSource> s,
               unique_ptr<Target> t)
        : ReplicationItem(id, "object", cfg, move(s), move(t)) {}
};

/* =========================
   8. Scope
========================= */

class Scope {
public:
    virtual vector<shared_ptr<ReplicationItem>>
    select(const vector<shared_ptr<ReplicationItem>>& items) = 0;

    virtual ~Scope() = default;
};

class AllScope : public Scope {
public:
    vector<shared_ptr<ReplicationItem>>
    select(const vector<shared_ptr<ReplicationItem>>& items) override {
        return items;
    }
};

class SingleScope : public Scope {
    shared_ptr<ReplicationItem> item;
public:
    SingleScope(shared_ptr<ReplicationItem> i)
        : item(move(i)) {}

    vector<shared_ptr<ReplicationItem>>
    select(const vector<shared_ptr<ReplicationItem>>& ) override {
        return {item};
    }
};

class TypeScope : public Scope {
    string type;
public:
    TypeScope(string t) : type(move(t)) {}

    vector<shared_ptr<ReplicationItem>>
    select(const vector<shared_ptr<ReplicationItem>>& items) override {

        vector<shared_ptr<ReplicationItem>> res;
        for (auto& i : items) {
            if (i->getType() == type)
                res.push_back(i);
        }
        return res;
    }
};

/* =========================
   9. Schedule (polymorphic)
========================= */

class Schedule {
public:
    virtual void start(function<void()> task) = 0;
    virtual ~Schedule() = default;
};

class ManualSchedule : public Schedule {
public:
    void start(function<void()> task) override {
        task();
    }
};

class ScheduledSchedule : public Schedule {
public:
    void start(function<void()> task) override {
        // simulate cron trigger
        task();
    }
};

class ContinuousSchedule : public Schedule {
public:
    void start(function<void()> task) override {
        // simulate loop
        task();
    }
};

/* =========================
   10. Retry Policy
========================= */

class RetryPolicy {
    int maxRetry;

public:
    RetryPolicy(int r) : maxRetry(r) {}

    template<typename Func>
    bool execute(Func f, Logger& logger, const string& id) {
        for (int i = 0; i <= maxRetry; i++) {
            try {
                f();
                return true;
            } catch (...) {
                logger.error("Retry failed " + id +
                             " attempt " + to_string(i));
                if (i == maxRetry) return false;
            }
        }
        return false;
    }
};

/* ======================
   11. Replication Project
   (no schedule here anymore)
========================= */

class ReplicationProject {
    ProjectConfig config;
    vector<shared_ptr<ReplicationItem>> items;

public:
    ReplicationProject(ProjectConfig cfg)
        : config(cfg) {}

    void addItem(shared_ptr<ReplicationItem> item) {
        items.push_back(item);
    }

    const vector<shared_ptr<ReplicationItem>>& getItems() const {
        return items;
    }

    const ProjectConfig& getConfig() const {
        return config;
    }
};

/* =========================
   12. Replication Job (core)
========================= */

class ReplicationJob {
    ProjectConfig config;
    vector<shared_ptr<ReplicationItem>> items;

    unique_ptr<Scope> scope;
    unique_ptr<Schedule> schedule;

    Logger logger;
    RetryPolicy retry;

public:
    ReplicationJob(ProjectConfig cfg,
                   vector<shared_ptr<ReplicationItem>> it,
                   unique_ptr<Scope> sc,
                   unique_ptr<Schedule> sch)
        : config(cfg),
          items(move(it)),
          scope(move(sc)),
          schedule(move(sch)),
          retry(cfg.retryCount) {}

    void start() {
        schedule->start([this]() {
            run();
        });
    }

private:
    void run() {
        logger.info("Job started");

        auto selected = scope->select(items);

        for (auto& item : selected) {

            logger.info("Processing " + item->getId());

            bool ok = retry.execute(
                [&]() {
                    item->replicate(config, logger);
                },
                logger,
                item->getId()
            );

            if (ok)
                logger.info("Success " + item->getId());
            else
                logger.error("Failed " + item->getId());
        }

        logger.info("Job finished");
    }
};

/* =========================
   13. MAIN
========================= */

int main() {
    ProjectConfig cfg{100, 2, true};

    ReplicationProject project(cfg);

    ItemConfig itemCfg;
    itemCfg.retryOverride = 3;

    auto item1 = make_shared<FileItem>(
        "file1",
        itemCfg,
        make_unique<FileDataSource>(),
        make_unique<ApplianceTarget>()
    );

    project.addItem(item1);

    // ✅ schedule now passed as parameter (IMPORTANT CHANGE)
    ReplicationJob job(
        project.getConfig(),
        project.getItems(),
        make_unique<AllScope>(),
        make_unique<ManualSchedule>()   // <-- no longer hardcoded in Project
    );

    job.start();

    return 0;
}