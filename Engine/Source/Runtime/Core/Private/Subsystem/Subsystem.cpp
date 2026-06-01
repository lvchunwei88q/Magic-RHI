#include "Subsystem/Subsystem.h"
#include "Subsystem/SubsystemContext.h"
#include "SubsystemContextImpl.h"

#include <cassert> // For assert
#include <algorithm>
namespace Core
{
	SubsystemContextImpl::SubsystemContextImpl()
	{
		// initialize
	}

	SubsystemContext* GetSubsystemContext()
	{
		return &SubsystemContextImpl::Get();
	}

	void SubsystemContextImpl::RegisterSubsystem(SubsystemContext::Context context) {
		// Register the subsystem in the system manager
		Context ctx;
		ctx.name = std::string(context.name);
		ctx.tags = std::string(context.tags);
		ctx.subsystem = (Subsystem*)context.subsystem;
		ctx.priority = context.priority;
		Derived.push_back(std::move(ctx));
		
	}

	SubsystemError SubsystemContextImpl::Init()
	{
		std::sort(Derived.begin(), Derived.end(),
			[](const Context& a, const Context& b) {
				return a.priority > b.priority;  // 优先级高的先初始化
			});

		for (size_t i = 0; i < Derived.size(); i++)
		{
			auto& it = Derived[i];

			for(auto& IC_Function :IC_Functions)
				if(IC_Function) IC_Function(it.name.c_str(), it.name.size(), i); // get name and allsize | currentindex

			if (!it.subsystem)
				assert(it.subsystem && "Subsystem is null during Init!");
			if (!it.subsystem->Init())
				return {false,it.name.c_str()};
		}
		return { true,nullptr };
	}
	void SubsystemContextImpl::Uninstall()
	{
		for (size_t i = Derived.size(); i-- > 0; )
		{
			auto& it = Derived[i];
			if (!it.subsystem)
				assert(it.subsystem && "Subsystem is null during uninstall!");
			it.subsystem->Uninstall();
		}
	}

	void SubsystemContextImpl::Notification(const char* target_tags, NotificationContext msg)
	{
		for (auto& Target : Derived)
		{
			if (Target.tags == target_tags) {
				Target.subsystem->Notification(msg); // msg
			}
		}
	}

	void SubsystemContextImpl::RegisterInitCallbackFunction(Init_Callback Func) {
		IC_Functions.push_back(std::move(Func));
	}

	size_t SubsystemContextImpl::Num() {
		return Derived.size();
	}
}