#include "Therminator3.hpp"
#include "FileSystem.hpp"
#include "StreamHelpers.hpp"
#include <memory>
#include "PrintHelpers.hpp"
#include "ModelHadronGas.hpp"
#include "ModelBlastWave.hpp"
#include "ModelBWA.hpp"
#include "ModelHydro2D.hpp"
#include "ModelHydro3D.hpp"
#include "ModelKrakowSFO.hpp"


namespace CAP
{

  Therminator3::Therminator3()
  :
  EventProcessor(),
  _createEvents(true),
  _importModel(false),
  _exportModel(false),
  _modelImportPath(""),
  _modelImportFileName(""),
  _modelExportPath(""),
  _modelExportFileName(""),
  _modelSelected("BW"),
  _useHistos(true),
  _createHistos(true),
  _importHistos(true),
  _importBaseName(""),
  _importPath(""),
  _importFileName(""),
  _exportHistos(true),
  _exportBaseName(""),
  _exportPath(""),
  _exportFileName("Therminator3"),
  _forceRewrite(true),
  _model(nullptr),
  _histograms()
  {
  appendClassName("Therminator3");
  setMinimumReportLevel(Object::kInfo);
  setName("Therminator3");
  setTitle("Therminator3");
  }

  Therminator3::Therminator3(const Therminator3 & source)
  :
  EventProcessor(source),
  _createEvents(source._createEvents),
  _importModel(source._importModel),
  _exportModel(source._exportModel),
  _modelImportPath(source._modelImportPath),
  _modelImportFileName(source._modelImportFileName),
  _modelExportPath(source._modelExportPath),
  _modelExportFileName(source._modelExportFileName),
  _modelSelected(source._modelSelected),
  _useHistos(source._useHistos),
  _createHistos(source._createHistos),
  _importHistos(source._importHistos),
  _importBaseName(source._importBaseName),
  _importPath(source._importPath),
  _importFileName(source._importFileName),
  _exportHistos(source._exportHistos),
  _exportBaseName(source._exportBaseName),
  _exportPath(source._exportPath),
  _exportFileName(source._exportFileName),
  _forceRewrite(source._forceRewrite),
  _model(source._model ? source._model->clone() : nullptr),   // its own copy: no shared, doubly deleted model
  _histograms(source._histograms)
  {   }

  Therminator3 & Therminator3::operator=(const Therminator3 & rhs)
  {
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _createEvents   = rhs._createEvents;
    _importModel    = rhs._importModel;
    _exportModel    = rhs._exportModel;
    _modelImportPath     = rhs._modelImportPath;
    _modelImportFileName = rhs._modelImportFileName;
    _modelExportPath     = rhs._modelExportPath;
    _modelExportFileName = rhs._modelExportFileName;
    _modelSelected  = rhs._modelSelected;
    _useHistos      = rhs._useHistos;
    _createHistos   = rhs._createHistos;
    _importHistos   = rhs._importHistos;
    _importBaseName = rhs._importBaseName;
    _importPath     = rhs._importPath;
    _importFileName = rhs._importFileName;
    _exportHistos   = rhs._exportHistos;
    _exportBaseName = rhs._exportBaseName;
    _exportPath     = rhs._exportPath;
    _exportFileName = rhs._exportFileName;
    _forceRewrite   = rhs._forceRewrite;
    delete _model;
    _model          = rhs._model ? rhs._model->clone() : nullptr;
    _histograms     = rhs._histograms;
    }
  return *this;
  }

  void Therminator3::setDefaultConfiguration()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  const String & taskName = EventProcessor::name();
  EventProcessor::setDefaultConfiguration();

  String objectType = "HYDRO_MODEL";
  _configuration.addProperty(createKey(taskName,objectType,"TYPE"), "BW");
  _configuration.addProperty(createKey(taskName,objectType,"EVENTS:CREATE"), true);
  // model parameters (Model::configure()): Therminator 2's blastwave.ini; lengths in fm, temperature and
  // chemical potentials in GeV
  _configuration.addProperty(createKey(taskName,objectType,"Type"),                         0);
  _configuration.addProperty(createKey(taskName,objectType,"Subtype"),                      0);
  _configuration.addProperty(createKey(taskName,objectType,"Thermodynamics:Temperature"),   0.1656);
  _configuration.addProperty(createKey(taskName,objectType,"Thermodynamics:MuB"),           0.0285);
  _configuration.addProperty(createKey(taskName,objectType,"Thermodynamics:MuI"),          -0.0009);
  _configuration.addProperty(createKey(taskName,objectType,"Thermodynamics:MuS"),           0.0069);
  _configuration.addProperty(createKey(taskName,objectType,"Thermodynamics:MuC"),           0.0);
  _configuration.addProperty(createKey(taskName,objectType,"pRapidityRange"),               4.0);
  _configuration.addProperty(createKey(taskName,objectType,"sRapidityRange"),               8.0);
  _configuration.addProperty(createKey(taskName,objectType,"rhoMax"),                       8.21);
  _configuration.addProperty(createKey(taskName,objectType,"TauI"),                         8.17);
  _configuration.addProperty(createKey(taskName,objectType,"TauC"),                         10.55);
  _configuration.addProperty(createKey(taskName,objectType,"tVelocity"),                    0.341);
  _configuration.addProperty(createKey(taskName,objectType,"Amplitude"),                    0.0);
  _configuration.addProperty(createKey(taskName,objectType,"Delay"),                        0.0);
  _configuration.addProperty(createKey(taskName,objectType,"BackFlow"),                     false);
  _configuration.addProperty(createKey(taskName,objectType,"NSamples"),                     100000);
  _configuration.addProperty(createKey(taskName,objectType,"MaxIntegrandSafety"),           1.2);
  _configuration.addProperty(createKey(taskName,objectType,"MultFractionMin"),              1.0);
  _configuration.addProperty(createKey(taskName,objectType,"MultFractionMax"),              1.0);
  _configuration.addProperty(createKey(taskName,objectType,"FluctuationType"),              0);
  _configuration.addProperty(createKey(taskName,objectType,"SystemName"),                   "PbPb");
  _configuration.addProperty(createKey(taskName,objectType,"SystemEnergy"),                 5020.0);
  _configuration.addProperty(createKey(taskName,objectType,"Centrality"),                   0.0);
  _configuration.addProperty(createKey(taskName,objectType,"CentralityMin"),                0.0);
  _configuration.addProperty(createKey(taskName,objectType,"CentralityMax"),                0.0);
  _configuration.addProperty(createKey(taskName,objectType,"ImpactParameter"),              0.0);
  _configuration.addProperty(createKey(taskName,objectType,"ImpactParameterMin"),           0.0);
  _configuration.addProperty(createKey(taskName,objectType,"ImpactParameterMax"),           0.0);
  // integrated hadron gas: read from (MULT:IMPORT) or written to (MULT:EXPORT) a text file
  _configuration.addProperty(createKey(taskName,objectType,"MULT:IMPORT"), false);
  _configuration.addProperty(createKey(taskName,objectType,"MULT:EXPORT"), false);
  _configuration.addProperty(createKey(taskName,objectType,"MULT:IMPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"MULT:IMPORT:FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"MULT:EXPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"MULT:EXPORT:FILE_NAME"),"NONE");

  // hypersurface file of the hydro models (HYDRO2D, HYDRO3D): a Therminator 2 XML hypersurface
  _configuration.addProperty(createKey(taskName,objectType,"HYPERS:IMPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"HYPERS:IMPORT:FILE_NAME"),"NONE");

  // parameters for event generation are handled in the base class.

  objectType = "HADRON_GAS";
  _configuration.addProperty(createKey(taskName,objectType,"temperature"), 0.160);
  _configuration.addProperty(createKey(taskName,objectType,"muBaryon"),    0.0);
  _configuration.addProperty(createKey(taskName,objectType,"muQ"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"muS"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"muC"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"muB"),         0.0);
  _configuration.addProperty(createKey(taskName,objectType,"muT"),         0.0);

  objectType = "HISTOGRAM";
  _configuration.addProperty(createKey(taskName,objectType,"USE"),             false);
  _configuration.addProperty(createKey(taskName,objectType,"BASE_NAME"),       "T3");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT"),          false);
  _configuration.addProperty(createKey(taskName,objectType,"CREATE"),          false);
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT"),          false);
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:BASE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),     "DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:BASE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),     "DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),true);
  _configuration.addProperty(createKey(taskName,objectType,"index_fill"),    true);
  _configuration.addProperty(createKey(taskName,objectType,"index_nbins"),   200);
  _configuration.addProperty(createKey(taskName,objectType,"index_min"),     0.0);
  _configuration.addProperty(createKey(taskName,objectType,"index_max"),     250.0);
  _configuration.addProperty(createKey(taskName,objectType,"mass_fill"),     true);
  _configuration.addProperty(createKey(taskName,objectType,"mass_nbins"),    10);
  _configuration.addProperty(createKey(taskName,objectType,"mass_min"),      0.0);
  _configuration.addProperty(createKey(taskName,objectType,"mass_max"),      4.0);
  }

  void Therminator3::configure(const Configuration & configuration)
  {
  EventProcessor::configure(configuration);
  String taskName = name();
  String typeName = "HYDRO_MODEL";

  _modelSelected = configuration.valueString(createKey(taskName,typeName,"TYPE"));
  if (_modelSelected.equalTo("HG")) // Hadron Gas W/O flow
    _model = new ModelHadronGas();
  else if (_modelSelected.equalTo("BW")) // Basic Blastwave
    _model = new ModelBlastWave();
  else if (_modelSelected.equalTo("BWA")) // Basic Blastwave Alternate
    _model = new ModelBWA();
  else if (_modelSelected.equalTo("HYDRO2D")) // Hydro2D
    _model = new ModelHydro2D();
  else if (_modelSelected.equalTo("HYDRO3D")) // Hydro3D
    _model = new ModelHydro3D();
  else if (_modelSelected.equalTo("SFO")) // Hydro SFO
    _model = new ModelKrakowSFO();
  else
    throw Exception("Unknown model selected",__FUNCTION__);
  _model->configure(taskName,typeName,configuration);
  _createEvents        = configuration.valueBool(  createKey(taskName,typeName,"EVENTS:CREATE"));
  _importModel         = configuration.valueBool(  createKey(taskName,typeName,"MULT:IMPORT"));
  _exportModel         = configuration.valueBool(  createKey(taskName,typeName,"MULT:EXPORT"));
  _modelImportPath     = FileSystem::configuredPath(configuration.valueString(createKey(taskName,typeName,"MULT:IMPORT:PATH")),"CAP_HISTOS_IMPORT_PATH");
  _modelImportFileName = configuration.valueString(createKey(taskName,typeName,"MULT:IMPORT:FILE_NAME"));
  _modelExportPath     = FileSystem::configuredPath(configuration.valueString(createKey(taskName,typeName,"MULT:EXPORT:PATH")),"CAP_HISTOS_EXPORT_PATH");
  _modelExportFileName = configuration.valueString(createKey(taskName,typeName,"MULT:EXPORT:FILE_NAME"));
  typeName = "HISTOGRAM";
  String importPathA;
  String exportPathA;
  _useHistos      = configuration.valueBool(  createKey(taskName,typeName,"USE"));
  _createHistos   = configuration.valueBool(  createKey(taskName,typeName,"CREATE"));
  _importHistos   = configuration.valueBool(  createKey(taskName,typeName,"IMPORT"));
  _importBaseName = configuration.valueString(createKey(taskName,typeName,"IMPORT:BASE_NAME"));
  importPathA    = configuration.valueString(createKey(taskName,typeName,"IMPORT:PATH"));
  _exportHistos   = configuration.valueBool(  createKey(taskName,typeName,"EXPORT"));
  _exportBaseName = configuration.valueString(createKey(taskName,typeName,"EXPORT:BASE_NAME"));
  exportPathA    = configuration.valueString(createKey(taskName,typeName,"EXPORT:PATH"));
  _forceRewrite   = configuration.valueBool(  createKey(taskName,typeName,"EXPORT:FORCE_REWRITE"));
  // "DEFAULT", absolute, "./..." or relative to $CAP_HISTOS_IMPORT_PATH / $CAP_HISTOS_EXPORT_PATH
  _importPath = FileSystem::configuredPath(importPathA,"CAP_HISTOS_IMPORT_PATH");
  _exportPath = FileSystem::configuredPath(exportPathA,"CAP_HISTOS_EXPORT_PATH");
  _histograms.configure(taskName,"HISTOGRAM",configuration,0);
  }


  void Therminator3::initialize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  if (nParticleDbs()==0)
    throw Exception("Therminator3 needs a particle database: set PARTICLE_DB:USE and PARTICLE_DB_0:NAME",__FUNCTION__);
  // the integrand maxima and the average multiplicities: read back from a file, or integrated here
  if (_importModel)
    {
    std::ifstream & inputFile = openInputAsciiFile(_modelImportPath,_modelImportFileName,".txt");
    _model->loadFromAscii(inputFile,db());
    inputFile.close();
    }
  else
    {
    _model->thermalHadronGas().loadHadrons(db());
    _model->calculateMultiplicities();
    }
  _histograms.create();
  }

  void Therminator3::execute()
  {
  _taskExecuted.increment();

  if (_createEvents)
    {
    // the event loop (EventIterator) resets the event and the particle factory before each execute()
    static const ParticleFilter acceptAll;
    const ParticleFilter & filter = particleFilters().empty() ? acceptAll : *particleFilters()[0];
    _model->generateEvent(event(),filter);
    _eventAccepted.increment();
    }
  }

  void Therminator3::finalize()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  EventProcessor::finalize();

  if (_exportModel)
    {
    std::ofstream & outputFile = openOutputAsciiFile(_modelExportPath,_modelExportFileName,".txt",true);
    _model->saveToAscii(outputFile);
    outputFile.close();
    }

  if (_exportHistos)
    {
    int temp = 1000*_model->temperature();
    _exportFileName = _exportBaseName + temp;
    std::unique_ptr<CapFile> outputFile(_forceRewrite ? CapFile::openRecreateCapFile(_exportPath,_exportFileName,true)
                                                     : CapFile::openNewCapFile(_exportPath,_exportFileName,true));
    _histograms.saveTo(*outputFile);
    outputFile->write();
    }
  // the model is used above (temperature, saveToAscii): delete it last
  delete _model;
  _model = nullptr;
  }


} // namespace CAP;
