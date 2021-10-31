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

int main(int argc, char ** argv)
{
    try {
        Init({
            .ApplicationName = DEMO_NAME,
            .ApplicationVersion = GetVersion(),
            .WindowTitle = DEMO_NAME " (" RYME_VERSION_STRING ")",
            .WindowSize = { 1024, 768 },
        });
        
        TestEntity * testEntity = new TestEntity();

        TestComponent * testComponent = new TestComponent();

        testEntity->AddComponent(testComponent);

        World::AddChild(testEntity);

        Run();

        Term();
    }
    catch (const std::exception& e) {
        Log(RYME_ANCHOR, "Exception {}", e.what());
    }

    fflush(stdout);

    return 0;
}