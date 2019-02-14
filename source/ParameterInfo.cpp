#include "Denn/ParameterInfo.h"

namespace Denn
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ParameterDomain
	ParameterDomain::ParameterDomain() {}

	ParameterDomain::ParameterDomain(const std::string& domain)
	: m_type(MONO)
	, m_domain(domain)
	{
	}

	ParameterDomain::ParameterDomain(const std::string& domain, const std::vector< std::string >& choises)
	: m_type(CHOISE)
	, m_domain(domain)
	, m_choises(choises)
	{
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParameterOwners::ParameterOwner::ParameterOwner()
	: m_filter_type(NONE)
	, m_owner_variable(nullptr)
	{
	}

	bool ParameterOwners::ParameterOwner::test(const ParameterInfo& owner) const
	{
		//ignore test
		if (!owner.has_an_associated_variable() && !m_owner_variable) return true;
		//associated to other owner
		if (owner.m_associated_variable != m_owner_variable) return false;
		//test
		switch (m_filter_type)
		{
		//all type
		default:
		case NONE:
		case ALL: return true;
		//not in
		case EXCEPT:
			for (const Variant& value : m_filter_values)
			{
				if (value.equal(owner.m_associated_variable->variant(), false)) return false;
			}
			return true;
		//in
		case ONLY:
			for (const Variant& value : m_filter_values)
			{
				if (value.equal(owner.m_associated_variable->variant(), false)) return true;
			}
			return false;
		}

	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParameterOwners::ParameterOwners()
	{
	}	
	ParameterOwners::ParameterOwners(const std::vector < ParameterOwner >& owners)
	: m_owners(owners)
	{
	}
	bool ParameterOwners::test(const ParameterInfo& test_owner) const
	{
		//ignore test
		if (!test_owner.has_an_associated_variable() && !m_owners.size()) return true;
		//test
		for (auto& the_owenr : m_owners) if (the_owenr.test(test_owner)) return true;
		//fail
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ParameterInfo::ParameterInfo() : m_associated_variable(nullptr) { }

	ParameterInfo::ParameterInfo
	(
		  const std::string&				   	description
		, const std::vector< std::string  >& 	arg_key
		, std::function< bool(Arguments&) >     action
		, const ParameterDomain&                domain
	)
	: m_associated_variable(nullptr)
	, m_description(description)
	, m_arg_key(arg_key)
	, m_action(action)
	, m_domain(domain)
	{
	}

	ParameterInfo::ParameterInfo
	(
		  const std::string&				  description
		, const ParameterOwners			      owener
		, const std::vector< std::string  >&  arg_key
		, std::function< bool(Arguments&) >   action
		, const ParameterDomain&              domain
	)
	: m_associated_variable(nullptr)
	, m_oweners(owener)
	, m_description(description)
	, m_arg_key(arg_key)
	, m_action(action)
	, m_domain(domain)
	{
	}

	bool ParameterInfo::has_an_associated_variable() const
	{
		return m_associated_variable != nullptr;
	}

	bool ParameterInfo::serializable() const
	{
		return has_an_associated_variable() && m_associated_variable->serializable();
	}
}