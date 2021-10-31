#include <Ryme/Ryme.hpp>

#include <Ryme/World.hpp>

using namespace ryme;

class TestEntity : public Entity
{
public:

    TestEntity() = default;

    virtual ~TestEntity() = default;

    void BeforeUpdate() override {
        Log(RYME_ANCHOR, "TestEntity::BeforeUpdate");
    }

    void OnUpdate() override {
        Log(RYME_ANCHOR, "TestEntity::OnUpdate");
    }

    void AfterUpdate() override {
        Log(RYME_ANCHOR, "TestEntity::AfterUpdate");
    }

    void BeforeRender() override {
        Log(RYME_ANCHOR, "TestEntity::BeforeRender");
    }

    void OnRender() override {
        Log(RYME_ANCHOR, "TestEntity::OnRender");
    }

    void AfterRender() override {
        Log(RYME_ANCHOR, "TestEntity::AfterRender");

        SetRunning(false);
    }

};

class TestComponent : public Component
{
public:

    TestComponent() = default;

    virtual ~TestComponent() = default;

    void AfterAttach() override {
        Log(RYME_ANCHOR, "TestComponent::AfterAttach");
    }

    void BeforeDetach() override {
        Log(RYME_ANCHOR, "TestComponent::BeforeDetach");
    }

    void OnUpdate() override {
        Log(RYME_ANCHOR, "TestComponent::OnUpdate");
    }

    void OnRender() override {
        Log(RYME_ANCHOR, "TestComponent::OnRender");
    }

};

class TestSystem : public System
{
public:

    TestSystem() = default;

    virtual ~TestSystem() = default;

    void OnUpdate() override {
        Log(RYME_ANCHOR, "TestSystem::OnUpdate");

        Log(RYME_ANCHOR, "TestSystem Collected {} TestEntity", _testEntityList.size());
        Log(RYME_ANCHOR, "TestSystem Collected {} TestComponent", _testComponentList.size());
    }

    void OnRender() override {
        Log(RYME_ANCHOR, "TestSystem::OnRender");
    }

    void OnEntityRegistered(Entity * entity, TypeIndex typeIndex) override {
        Log(RYME_ANCHOR, "TestSystem::OnEntityRegistered ({})", typeIndex.name());

        if (typeIndex.hash_code() == typeid(TestEntity).hash_code()) {
            _testEntityList.push_back(static_cast<TestEntity *>(entity));
        }
    }

    void OnEntityUnregistered(Entity * entity) override {
        Log(RYME_ANCHOR, "TestSystem::OnEntityUnregistered");

        auto it = std::find(_testEntityList.begin(), _testEntityList.end(), entity);
        if (it != _testEntityList.end()) {
            _testEntityList.erase(it);
        }
    }

    void OnComponentRegistered(Component * component, TypeIndex typeIndex) override {
        Log(RYME_ANCHOR, "TestSystem::OnComponentRegistered ({})", typeIndex.name());

        if (typeIndex.hash_code() == typeid(TestComponent).hash_code()) {
            _testComponentList.push_back(static_cast<TestComponent *>(component));
        }
    }

    void OnComponentUnregistered(Component * component) override {
        Log(RYME_ANCHOR, "TestSystem::OnComponentUnregistered");

        auto it = std::find(_testComponentList.begin(), _testComponentList.end(), component);
        if (it != _testComponentList.end()) {
            _testComponentList.erase(it);
        }
    }


private:

    List<TestEntity *> _testEntityList;

    List<TestComponent *> _testComponentList;

};

int main(int argc, char ** argv)
{
    try {
        Init({
            .ApplicationName = DEMO_NAME,
            .ApplicationVersion = GetVersion(),
            .WindowTitle = DEMO_NAME " (" RYME_VERSION_STRING ")",
            .WindowSize = { 1024, 768 },
        });
        
        World::AddSystem(new TestSystem());

        for (int i = 0; i < 5; ++i) {
            World::GetRootEntity()
                ->AddEntity(new TestEntity())
                ->AddComponent(new TestComponent());

            World::GetRootEntity()
                ->AddEntity(new Entity())
                ->AddComponent(new Component());
        }

        Run();

        Term();
    }
    catch (const std::exception& e) {
        Log(RYME_ANCHOR, "Exception {}", e.what());
    }

    fflush(stdout);

    return 0;
}