#include "Misc/AutomationTest.h"
#include "AutomationDriverCommon.h"
#include "AutomationDriverTypeDefs.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"


BEGIN_DEFINE_SPEC(AutomationSpec, "TestGroup.TestSubGroup.MyProject Specs", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
FString RunOrder;
END_DEFINE_SPEC(AutomationSpec)
void AutomationSpec::Define()
{
    Describe("A custom spec", [this]()
        {
            BeforeEach([this]()
                {
                    RunOrder = TEXT("A");
                });

            It("runs for MyProject", [this]()
                {
                    TestEqual("RunOrder", RunOrder, TEXT("A"));
                });

            AfterEach([this]()
                {
                    RunOrder += TEXT("Z");
                    TestEqual("RunOrder", RunOrder, TEXT("AZ"));
                });
        });
}