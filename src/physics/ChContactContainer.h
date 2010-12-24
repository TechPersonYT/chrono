#ifndef CHCONTACTCONTAINER_H
#define CHCONTACTCONTAINER_H

///////////////////////////////////////////////////
//
//   ChContactContainer.h
//
//   Class for container of many contacts, as CPU
//   typical linked list of ChContact objects (that
//   is contacts between two 6DOF bodies)
//
//   HEADER file for CHRONO,
//	 Multibody dynamics engine
//
// ------------------------------------------------
// 	 Copyright:Alessandro Tasora / DeltaKnowledge
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////



#include "physics/ChContactContainerBase.h"
#include "physics/ChContact.h"
#include "physics/ChContactRolling.h"
#include <list>

namespace chrono
{


///
/// Class representing a container of many contacts, 
/// implemented as a typical linked list of ChContact 
/// objects (that is, contacts between two 6DOF bodies).
/// It also contains rolling contact objects, if needed.
/// This is the default contact container used in most
/// cases.
///

class ChApi ChContactContainer : public ChContactContainerBase {

	CH_RTTI(ChContactContainer,ChContactContainerBase);

protected:
				//
	  			// DATA
				//

	std::list<ChContact*>   contactlist; 

	int n_added;

	std::list<ChContact*>::iterator lastcontact;


	std::list<ChContactRolling*>   contactlist_roll; 

	int n_added_roll;

	std::list<ChContactRolling*>::iterator lastcontact_roll;

public:
				//
	  			// CONSTRUCTORS
				//

	ChContactContainer ();

	virtual ~ChContactContainer ();


				//
	  			// FUNCTIONS
				//
					/// Tell the number of added contacts
	virtual int GetNcontacts  () {return n_added + n_added_roll;}

					/// Remove (delete) all contained contact data.
	virtual void RemoveAllContacts();

					/// The collision system will call BeginAddContact() before adding
					/// all contacts (for example with AddContact() or similar). Instead of
					/// simply deleting all list of the previous contacts, this optimized implementation
					/// rewinds the link iterator to begin and tries to reuse previous contact objects
					/// until possible, to avoid too much allocation/deallocation.
	virtual void BeginAddContact();

					/// Add a contact between two frames.
	virtual void AddContact(const collision::ChCollisionInfo& mcontact);

					/// The collision system will call BeginAddContact() after adding
					/// all contacts (for example with AddContact() or similar). This optimized version
					/// purges the end of the list of contacts that were not reused (if any).
	virtual void EndAddContact();

					/// Scans all the contacts and for each contact exacutes the ReportContactCallback()
					/// function of the user object inherited from ChReportContactCallback.
					/// Child classes of ChContactContainerBase should try to implement this (although
					/// in some highly-optimized cases as in ChContactContainerGPU it could be impossible to
					/// report all contacts).
	virtual void ReportAllContacts(ChReportContactCallback* mcallback);




					/// Tell the number of scalar bilateral constraints (actually, friction
					/// constraints aren't exactly as unilaterals, but count them too)
	virtual int GetDOC_d  () {return (n_added * 3) + (n_added_roll * 6);}

					/// In detail, it computes jacobians, violations, etc. and stores 
					/// results in inner structures of contacts.
	virtual void Update (double mtime);			
	
				//
				// LCP INTERFACE
				//

	virtual void InjectConstraints(ChLcpSystemDescriptor& mdescriptor);
	virtual void ConstraintsBiReset();
	virtual void ConstraintsBiLoad_C(double factor=1., double recovery_clamp=0.1, bool do_clamp=false);
	//virtual void ConstraintsBiLoad_Ct(double factor=1.) {};
	virtual void ConstraintsLoadJacobians();
	virtual void ConstraintsLiLoadSuggestedSpeedSolution();
	virtual void ConstraintsLiLoadSuggestedPositionSolution();
	virtual void ConstraintsLiFetchSuggestedSpeedSolution();
	virtual void ConstraintsLiFetchSuggestedPositionSolution();
	virtual void ConstraintsFetch_react(double factor=1.);


};




//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


} // END_OF_NAMESPACE____

#endif
