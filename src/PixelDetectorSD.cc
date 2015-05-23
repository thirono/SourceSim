#include "PixelDetectorSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

#include <iostream>

const G4int maxHits = 20; // maximum number of hits in one event that are stored

PixelDetectorSD::PixelDetectorSD(const G4String& name, const G4String& hitsCollectionName, G4int nPixel) :
		G4VSensitiveDetector(name), fHitsMap(0), fNpixel(nPixel)
{
	collectionName.insert(hitsCollectionName);
}

PixelDetectorSD::~PixelDetectorSD()
{
}

void PixelDetectorSD::Initialize(G4HCofThisEvent* hitCollection)
{
	// Create hits collection
	fHitsMap = new SiHitsMap(SensitiveDetectorName, collectionName[0]);

	// Add this collection in hce
	G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
	hitCollection->AddHitsCollection(hcID, fHitsMap);

	// Create hits
	// fNpixel for pixel + one more for total sums
	for (G4int i = 0; i < maxHits; i++) {
		fHitsMap->add(i, new SiHit());
	}
}

G4bool PixelDetectorSD::ProcessHits(G4Step* step, G4TouchableHistory* history)
{
	// energy deposit
	G4double edep = step->GetTotalEnergyDeposit();

	// step length
	G4double stepLength = 0.;
	if (step->GetTrack()->GetDefinition()->GetPDGCharge() != 0.) {
		stepLength = step->GetStepLength();
	}

	if (edep == 0. && stepLength == 0.)
		return false;

	G4TouchableHistory* touchable = (G4TouchableHistory*) (step->GetPreStepPoint()->GetTouchable());

	if (!touchable) {
		G4ExceptionDescription msg;
		msg << "Cannot access touchable !";
		G4Exception("PixelDetectorSD::ProcessHits()", "MyCode0004", FatalException, msg);
	}

	// Get hit pixel id
	G4int iVolume = touchable->GetReplicaNumber(0);

	// Get hit accounting data for this pixel
	(*fHitsMap)[iVolume]->Add(edep, stepLength);

	// Get hit for total accounting
	(*fHitsMap)[fNpixel]->Add(edep, stepLength);

	return true;
}

void PixelDetectorSD::EndOfEvent(G4HCofThisEvent* hitCollection)
{
	if (verboseLevel > 1) {
		G4int nofHits = fHitsMap->entries();
		G4cout << G4endl<< "--------> Hits Collection: in this event they are " << nofHits << " hits in the pixel detector: " << G4endl;
		for (G4int i = 0; i < nofHits; i++)
			(*fHitsMap)[i]->Print();
	}
}
