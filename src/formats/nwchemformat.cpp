/**********************************************************************
Copyright (C) 2001-2005 by Geoffrey R. Hutchison
Some portions Copyright (C) 2004 by Chris Morley
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/

#include "mol.h"
#include "obconversion.h"

using namespace std;
namespace OpenBabel
{

class NWChemOutputFormat : public OBFormat
{
public:
    //Register this format type ID
    NWChemOutputFormat()
    {
        OBConversion::RegisterFormat("nwo",this);
    }

    virtual const char* Description() //required
    {
        return
            "NWChem output format\n \
            No comments yet\n \
            ";
    };

    virtual const char* SpecificationURL(){return
            "http://www.emsl.pnl.gov/docs/nwchem/";}; //optional

    //Flags() can return be any the following combined by | or be omitted if none apply
    // NOTREADABLE  READONEONLY  NOTWRITABLE  WRITEONEONLY
    virtual unsigned int Flags()
    {
        return READONEONLY | NOTWRITABLE;
    };

    ////////////////////////////////////////////////////
    /// The "API" interface functions
    virtual bool ReadMolecule(OBBase* pOb, OBConversion* pConv);

    ////////////////////////////////////////////////////
    /// The "Convert" interface functions
    virtual bool ReadChemObject(OBConversion* pConv)
    {
        OBMol* pmol = new OBMol;
        bool ret=ReadMolecule(pmol,pConv);
        if(ret) //Do transformation and return molecule
            pConv->AddChemObject(pmol->DoTransformations(pConv->GetGeneralOptions()));
        else
            pConv->AddChemObject(NULL);
        return ret;
    };

};

//Make an instance of the format class
NWChemOutputFormat theNWChemOutputFormat;

class NWChemInputFormat : public OBFormat
{
public:
    //Register this format type ID
    NWChemInputFormat()
    {
        OBConversion::RegisterFormat("nw",this);
    }

    virtual const char* Description() //required
    {
        return
            "NWChem input format\n \
            No comments yet\n \
            ";
    };

    virtual const char* SpecificationURL(){return
            "http://www.emsl.pnl.gov/docs/nwchem/";}; //optional

    //Flags() can return be any the following combined by | or be omitted if none apply
    // NOTREADABLE  READONEONLY  NOTWRITABLE  WRITEONEONLY
    virtual unsigned int Flags()
    {
      return NOTREADABLE | WRITEONEONLY;
    };

    ////////////////////////////////////////////////////
    /// The "API" interface functions
    virtual bool WriteMolecule(OBBase* pOb, OBConversion* pConv);

    ////////////////////////////////////////////////////
    /// The "Convert" interface functions
    virtual bool WriteChemObject(OBConversion* pConv)
    {
        //Retrieve the target OBMol
        OBBase* pOb = pConv->GetChemObject();
        OBMol* pmol = dynamic_cast<OBMol*> (pOb);
        bool ret=false;
        if(pmol)
            ret=WriteMolecule(pmol,pConv);
        delete pOb;
        return ret;
    };
};

//Make an instance of the format class
NWChemInputFormat theNWChemInputFormat;


/////////////////////////////////////////////////////////////////
bool NWChemOutputFormat::ReadMolecule(OBBase* pOb, OBConversion* pConv)
{

    OBMol* pmol = dynamic_cast<OBMol*>(pOb);
    if(pmol==NULL)
        return false;

    //Define some references so we can use the old parameter names
    istream &ifs = *pConv->GetInStream();
    OBMol &mol = *pmol;
    const char* title = pConv->GetTitle();

    char buffer[BUFF_SIZE];
    string str;
    double x,y,z;
    OBAtom *atom;
    vector<string> vs;

    mol.BeginModify();
    while	(ifs.getline(buffer,BUFF_SIZE))
    {
        if(strstr(buffer,"Output coordinates") != NULL)
        {
            // mol.EndModify();
            mol.Clear();
            mol.BeginModify();
            ifs.getline(buffer,BUFF_SIZE);	// blank
            ifs.getline(buffer,BUFF_SIZE);	// column headings
            ifs.getline(buffer,BUFF_SIZE);	// ---- ----- ----
            ifs.getline(buffer,BUFF_SIZE);
            tokenize(vs,buffer);
            while (vs.size() == 6)
            {
                atom = mol.NewAtom();
                x = atof((char*)vs[3].c_str());
                y = atof((char*)vs[4].c_str());
                z = atof((char*)vs[5].c_str());
                atom->SetVector(x,y,z); //set coordinates

                //set atomic number
                atom->SetAtomicNum(etab.GetAtomicNum(vs[1].c_str()));

                if (!ifs.getline(buffer,BUFF_SIZE))
                    break;
                tokenize(vs,buffer);
            }
        } // if "output coordinates"
    } // while
    mol.ConnectTheDots();
    mol.PerceiveBondOrders();

    mol.EndModify();

    mol.SetTitle(title);
    return(true);
}

////////////////////////////////////////////////////////////////

bool NWChemInputFormat::WriteMolecule(OBBase* pOb, OBConversion* pConv)
{
    OBMol* pmol = dynamic_cast<OBMol*>(pOb);
    if(pmol==NULL)
        return false;

    //Define some references so we can use the old parameter names
    ostream &ofs = *pConv->GetOutStream();
    OBMol &mol = *pmol;
    const char *dimension = pConv->GetDimension();

    unsigned int i;
    char buffer[BUFF_SIZE];

    ofs << "start molecule" << endl << endl;
    ofs << "title " << endl << " " << mol.GetTitle() << endl << endl;

    ofs << "geometry units angstroms print xyz autosym" << endl;

    OBAtom *atom;
    for(i = 1;i <= mol.NumAtoms(); i++)
    {
        atom = mol.GetAtom(i);
        sprintf(buffer,"%3s%15.5f%15.5f%15.5f",
                etab.GetSymbol(atom->GetAtomicNum()),
                atom->GetX(),
                atom->GetY(),
                atom->GetZ());
        ofs << buffer << endl;
    }

    ofs << "end" << endl;

    return(true);
}

} //namespace OpenBabel
