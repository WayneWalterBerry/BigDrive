#include "pch.h"
#include "CppUnitTest.h"

// System
#include <windows.h>
#include <wtypes.h>

#include "COMAdminCatalog.h"
#include "ApplicationCollection.h"
#include "ComponentCollection.h"
#include "Application.h"
#include "Component.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigDriveClientTest
{
    TEST_CLASS(ComponentCollectionTests)
    {
    public:

        TEST_METHOD(InitializeAndGetCountTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection = nullptr;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lAppCount = 0;
            hr = pApplicationCollection->GetCount(lAppCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lAppCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");

            ComponentCollection* pComponentCollection = nullptr;
            hr = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetComponentCollection() failed.");
            Assert::IsNotNull(pComponentCollection, L"GetComponentCollection() returned nullptr.");

            LONG lComponentCount = 0;
            hr = pComponentCollection->GetCount(lComponentCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lComponentCount > 0, L"Expected at least one component.");

            // Cleanup
            delete pComponentCollection;
            delete pApplication;
            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(GetItemAndCloneTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection = nullptr;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lAppCount = 0;
            hr = pApplicationCollection->GetCount(lAppCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lAppCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");

            ComponentCollection* pComponentCollection = nullptr;
            hr = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetComponentCollection() failed.");
            Assert::IsNotNull(pComponentCollection, L"GetComponentCollection() returned nullptr.");

            hr = pComponentCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lComponentCount = 0;
            hr = pComponentCollection->GetCount(lComponentCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lComponentCount > 0, L"Expected at least one component.");

            Component* pComponent = nullptr;
            hr = pComponentCollection->GetItem(0, &pComponent);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");
            Assert::IsNotNull(pComponent, L"GetItem() returned nullptr.");

            Component* pClonedComponent = nullptr;
            hr = pComponent->Clone(&pClonedComponent);
            Assert::IsTrue(SUCCEEDED(hr), L"Clone() failed.");
            Assert::IsNotNull(pClonedComponent, L"Clone() returned nullptr.");

            // Cleanup
            delete pClonedComponent;
            delete pComponent;
            delete pComponentCollection;
            delete pApplication;
            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }

        TEST_METHOD(GetCLSIDTest)
        {
            HRESULT hr = S_OK;

            COMAdminCatalog* pCOMAdminCatalog = nullptr;
            hr = COMAdminCatalog::Create(&pCOMAdminCatalog);
            Assert::IsTrue(SUCCEEDED(hr), L"Create() failed.");

            ApplicationCollection* pApplicationCollection = nullptr;
            hr = pCOMAdminCatalog->GetApplicationsCollection(&pApplicationCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetApplicationsCollection() failed.");

            hr = pApplicationCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lAppCount = 0;
            hr = pApplicationCollection->GetCount(lAppCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lAppCount > 0, L"Expected at least one application.");

            Application* pApplication = nullptr;
            hr = pApplicationCollection->GetItem(0, &pApplication);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");

            ComponentCollection* pComponentCollection = nullptr;
            hr = pCOMAdminCatalog->GetComponentCollection(pApplication, &pComponentCollection);
            Assert::IsTrue(SUCCEEDED(hr), L"GetComponentCollection() failed.");
            Assert::IsNotNull(pComponentCollection, L"GetComponentCollection() returned nullptr.");

            hr = pComponentCollection->Initialize();
            Assert::IsTrue(SUCCEEDED(hr), L"Initialize() failed.");

            LONG lComponentCount = 0;
            hr = pComponentCollection->GetCount(lComponentCount);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCount() failed.");
            Assert::IsTrue(lComponentCount > 0, L"Expected at least one component.");

            Component* pComponent = nullptr;
            hr = pComponentCollection->GetItem(0, &pComponent);
            Assert::IsTrue(SUCCEEDED(hr), L"GetItem() failed.");
            Assert::IsNotNull(pComponent, L"GetItem() returned nullptr.");

            CLSID clsid;
            hr = pComponent->GetCLSID(clsid);
            Assert::IsTrue(SUCCEEDED(hr), L"GetCLSID() failed.");

            // Cleanup
            delete pComponent;
            delete pComponentCollection;
            delete pApplication;
            delete pApplicationCollection;
            delete pCOMAdminCatalog;
        }
    };
}
