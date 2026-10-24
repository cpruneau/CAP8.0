/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__HistogramFolder
#define CAP__HistogramFolder
#include <vector>
#include "String.hpp"
#include "CapFile.hpp"
#include "Configuration.hpp"
#include "Exceptions.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include <fstream>

namespace CAP
{
  using MATH::Histogram;
  using MATH::Profile;
  class Task;

  //!
  //! @brief Group of histograms (CAP::MATH::Histogram) produced or used by a task, held in a Folder.
  //!
  //! The group is a Folder: it owns its histograms, which are entries named after the histograms. Histograms are added
  //! with append() or the createHistogram() / createProfile() helpers, and retrieved with histogram(name) or
  //! histograms(). Copying a group copies its histograms (deep copy).
  //!
  //! Derived classes typically keep pointers to their histograms (e.g. h_n1). Those pointers refer to objects owned by
  //! the group; after a copy or an assignment, set them again from the new group, e.g. h_n1 = histogramLike(source.h_n1).
  //!
  //! Files: the histograms are saved to, and loaded from, a folder: in a job, the folder of the combination of filters
  //! of the group, in the folder of its task, in the output (input) file of the job (see Task::saveHistograms()).
  //! saveTo() copies them into the folder, and importHistogram() copies one back. Their names are short (e.g. n1_pt):
  //! the folders tell the groups apart.
  //!
  //! Profiles are MATH::Profile objects (createProfile(), importProfile()), filled with fill(x, value, weight); their
  //! bin values are the weighted means. They are stored, copied and saved as profiles.
  //!
  //! This class replaces RootHistogramGroup (ROOT TH1/TH2/TH3, TFile) for classes converted to CAP histograms.
  //!
  class HistogramFolder : public Folder
  {
  public:

  //! @brief The histograms of the group (CAP::MATH::Histogram); also usable by derived classes.
  using Histogram = MATH::Histogram;
  using Profile   = MATH::Profile;

  protected:

  String _histogramBaseName;  //!< prefix of the names of the histograms of the group
  Task * _parentTask;         //!< task using this group (not owned)

  public:

  //! @name Construction
  //! @{


  //! @brief Constructor. @param name name of the group
  HistogramFolder(const String & name="HistogramFolder");
  //! @brief Copy constructor: copies the histograms. @param source group to copy
  HistogramFolder(const HistogramFolder & source);
  //! @brief Assignment: replaces the histograms by copies of those of rhs. @param rhs group to copy @return this group
  HistogramFolder & operator=(const HistogramFolder & rhs);
  //! @brief Destructor: the histograms are deleted with the group.
  virtual ~HistogramFolder() {}
  //! @brief Copy of this group (derived classes override it to copy their own type). @return new group, owned by the caller
  virtual HistogramFolder * clone() const { return new HistogramFolder(*this); }

  //! @}


  //! @name Task interface
  //! @{


  //!
  //! @brief Read the configuration of the group: for types other than HISTOGRAM... (corrections), the base name of the
  //! histograms (taskName:objectType_index:BASE_NAME); the histograms of the analyzers have short names.
  //! @param taskName name of the task @param objectType type of object (e.g. "HISTOGRAM") @param configuration configuration
  //! @param index index of the group within the task
  //!
  virtual void configure(const String & taskName, const String & objectType,
                         const Configuration & configuration, unsigned int index);
  //! @brief Initialization hook (nothing to do here).
  virtual void initialize() {}
  //! @brief Create the histograms of the group. @throws Exception here: derived classes implement it
  virtual void create();
  //! @brief Load the histograms of the group from a folder. @param file folder (e.g. that of the group in a CapFile)
  //! @throws Exception here: derived classes implement it (typically with importHistogram())
  virtual void loadFrom(const Folder & file);
  //!
  //! @brief Copy the histograms of the group into a folder, replacing entries with the same names. @param file folder
  //!
  virtual void saveTo(Folder & file) const;
  //! @brief Multiply all histograms by a factor (see scaleHistograms()). @param factor factor
  virtual void scaleObject(double factor) { scaleHistograms(factor); }
  //! @brief Reset the content of all histograms (their binning is kept).
  void reset() override;
  //! @brief Delete all histograms (and sub-folders) of the group.
  void clear() override { Folder::clear(); }
  //! @brief Print the group: name, base name and its histograms.
  void print() const override;

  //! @brief Task using this group. @return task, or nullptr
  Task * parentTask() const { return _parentTask; }
  //! @brief Set the task using this group. @param parentTask task (not owned)
  void setParentTask(Task * parentTask) { _parentTask = parentTask; }
  //! @brief Name of the task using this group. @return name @throws Exception if there is no task
  const String parentName() const;
  //! @brief Full path of the task using this group. @return path @throws Exception if there is no task
  const String parentPathName() const;
  //! @brief Base name of the histograms. @return base name
  const String & histogramBaseName() const { return _histogramBaseName; }
  //! @brief Base name of the histograms (same as histogramBaseName()). @return base name
  const String & baseName() const { return _histogramBaseName; }
  //! @brief Set the base name of the histograms. @param name base name
  void setHistogramBaseName(const String & name) { _histogramBaseName = name; }

  //! @}


  //! @name Histograms
  //! @{


  //! @brief Number of histograms in the group. @return number of histograms
  unsigned int nHistograms() const;
  //! @brief Histograms of the group, in the order they were added. @return pointers (owned by the group)
  std::vector<Histogram *> histograms() const;
  //! @brief Histogram of the given name. @param name name @return histogram (owned by the group), or nullptr if none
  Histogram * histogram(const String & name) const { return get<Histogram>(name); }
  //!
  //! @brief Histogram (or Profile) of this group with the same name and type as h (used to set pointers after a copy).
  //! @param h histogram of another group (may be null) @return histogram of this group, or nullptr if h is null
  //! @throws Exception if this group has no histogram of type H with that name
  //! @tparam H Histogram or Profile
  //!
  template <class H>
  H * histogramLike(const H * h) const
  {
  if (!h) return nullptr;
  H * mine = dynamic_cast<H *>(histogram(h->name()));
  if (!mine) throw Exception(String("Group has no histogram of that type named ")+h->name(),"HistogramFolder::histogramLike()");
  return mine;
  }
  //!
  //! @brief Add a histogram to the group (the group takes ownership); its entry is named after the histogram.
  //! @param h histogram @return h
  //! @throws Exception if h is null or the group already has an entry with that name (h is then deleted)
  //! A Profile is stored as a Profile.
  //!
  Histogram * append(Histogram * h);

  //! @}


  //! @name Creating histograms
  //! The histograms are created empty, added to the group, and returned. Axis titles are stored in the binning
  //! (Partition::dimensionTitle()); the title of the dependent value is stored as the title of the histogram.
  //! storageOption selects the precision of the bin values: 0 or 1 for float, 2 for double (as the storage options of
  //! the ROOT version; see Histogram::precisionFromStorageOption()).
  //! @{


  //! @brief Create a rank 1 histogram. @param name name @param n_x number of bins @param min_x lower edge @param max_x upper edge
  //! @param title_x title of the x axis @param title_y title of the value @param storageOption 0 or 1: float, 2: double
  //! @return histogram @throws Exception if the binning is invalid
  Histogram * createHistogram(const String & name,
                              unsigned int n_x, double min_x, double max_x,
                              const String & title_x="", const String & title_y="", int storageOption=1);
  //! @brief Create a rank 2 histogram. @param name name @param n_x,min_x,max_x binning of x @param n_y,min_y,max_y binning of y
  //! @param title_x title of x @param title_y title of y @param title_z title of the value
  //! @param storageOption 0 or 1: float, 2: double @return histogram
  Histogram * createHistogram(const String & name,
                              unsigned int n_x, double min_x, double max_x,
                              unsigned int n_y, double min_y, double max_y,
                              const String & title_x="", const String & title_y="", const String & title_z="",
                              int storageOption=1);
  //! @brief Create a rank 3 histogram. @param name name @param n_x,min_x,max_x binning of x @param n_y,min_y,max_y binning of y
  //! @param n_z,min_z,max_z binning of z @param title_x title of x @param title_y title of y @param title_z title of z
  //! @param title_w title of the value @param storageOption 0 or 1: float, 2: double @return histogram
  Histogram * createHistogram(const String & name,
                              unsigned int n_x, double min_x, double max_x,
                              unsigned int n_y, double min_y, double max_y,
                              unsigned int n_z, double min_z, double max_z,
                              const String & title_x="", const String & title_y="", const String & title_z="",
                              const String & title_w="", int storageOption=1);
  //! @brief Create a rank 1 profile (fill(x,value,weight); mean values as bin values, see MATH::Profile).
  //! @param name name @param n_x,min_x,max_x binning of x @param title_x title of x @param title_y title of the mean value
  //! @return profile
  Profile * createProfile(const String & name,
                          unsigned int n_x, double min_x, double max_x,
                          const String & title_x="", const String & title_y="");
  //! @brief Create a rank 2 profile (fill(x,y,value,weight)).
  //! @param name name @param n_x,min_x,max_x binning of x @param n_y,min_y,max_y binning of y
  //! @param title_x title of x @param title_y title of y @param title_z title of the mean value @return profile
  Profile * createProfile(const String & name,
                          unsigned int n_x, double min_x, double max_x,
                          unsigned int n_y, double min_y, double max_y,
                          const String & title_x="", const String & title_y="", const String & title_z="");

  //! @}


  //! @name Files
  //! @{


  //!
  //! @brief Copy a histogram from the current folder of a file into the group (replacing one with the same name).
  //! A profile is copied as a profile.
  //! @param file file (read with CapFile::read()) @param name name of the histogram @return the copy, owned by the group
  //! @throws FileException if the file has no histogram with that name
  //!
  Histogram * importHistogram(const Folder & file, const String & name);

  //!
  //! @brief Copy a profile from the current folder of a file into the group (replacing one with the same name).
  //! @param file file @param name name of the profile @return the copy, owned by the group
  //! @throws FileException if the file has no profile with that name
  //!
  Profile * importProfile(const Folder & file, const String & name);

  //!
  //! @brief Copy all the histograms and profiles of the current folder of a file into the group (replacing those with
  //! the same names). Other entries and sub-folders are ignored.
  //! @param file file (read with CapFile::read()) @return number of histograms copied
  //!
  unsigned int importAllHistograms(const Folder & file);

  //! @}


  //! @name Interface used by ManagedObjects
  //! Histogram groups are stored in CAP files only: the text-file functions do nothing.
  //! @{

  //! @brief Does nothing. @param file text file
  virtual void loadFromAscii(std::ifstream & file __attribute__((unused))) {}
  //! @brief Does nothing. @param file text file
  virtual void saveToAscii(std::ofstream & file __attribute__((unused))) {}
  //! @}


  //! @name Operations on all histograms
  //! Operations between groups match histograms by name: every histogram of this group (or of the result group) must
  //! have a histogram with the same name and a compatible binning in the other groups (Exception otherwise).
  //! @{


  //! @brief Multiply all histograms by a. @param a factor
  void scaleHistograms(double a);
  //! @brief Divide every histogram by the volume of its bins, times scale (densities). Do not use on profiles.
  //! @param scale additional factor
  void scaleAllHistoByBinWidth(double scale=1.0);
  //! @brief Whether rhs has, for every histogram of this group, one with the same name and binning. @param rhs other group @return true if so
  bool sameSizeAs(const HistogramFolder & rhs) const;
  //! @brief Add rhs histogram by histogram. @param rhs other group @return this group
  HistogramFolder & operator+=(const HistogramFolder & rhs);
  //! @brief Subtract rhs histogram by histogram. @param rhs other group @return this group
  HistogramFolder & operator-=(const HistogramFolder & rhs);

  //! @brief sum = a1*g1, histogram by histogram. @param a1 coefficient @param g1 group @param sum result (existing histograms)
  static void sumHistogramGroups(double a1, const HistogramFolder & g1, HistogramFolder & sum);
  //! @brief sum = a1*g1 + a2*g2. @param a1,g1 first term @param a2,g2 second term @param sum result
  static void sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                 HistogramFolder & sum);
  //! @brief sum = a1*g1 + a2*g2 + a3*g3. @param a1,g1 first term @param a2,g2 second term @param a3,g3 third term @param sum result
  static void sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                 double a3, const HistogramFolder & g3, HistogramFolder & sum);
  //! @brief sum = a1*g1 + a2*g2 + a3*g3 + a4*g4. @param a1,g1 first @param a2,g2 second @param a3,g3 third @param a4,g4 fourth @param sum result
  static void sumHistogramGroups(double a1, const HistogramFolder & g1, double a2, const HistogramFolder & g2,
                                 double a3, const HistogramFolder & g3, double a4, const HistogramFolder & g4,
                                 HistogramFolder & sum);
  //! @brief ratio = (a1*numerator)/(a2*denominator), histogram by histogram.
  //! @param a1 coefficient of the numerator @param numerator group @param a2 coefficient of the denominator
  //! @param denominator group @param ratio result (existing histograms)
  static void divideHistogramsLists(double a1, const HistogramFolder & numerator,
                                    double a2, const HistogramFolder & denominator, HistogramFolder & ratio);
  //!
  //! @brief Running average and spread over sub-samples: update every bin of this group (the average of the previous
  //! sub-samples, with weight sumWeights) with the bin of group (the new sub-sample, with weight weight). The bin
  //! uncertainty becomes the spread, divided by sqrt(n) if n>0 (uncertainty of the mean).
  //! @param group new sub-sample @param sumWeights total weight of the previous sub-samples @param weight weight of the new one
  //! @param n number of sub-samples (n<=0: keep the spread)
  //!
  void squareDifferenceLists(const HistogramFolder & group, double sumWeights, double weight, int n);

  //! @}

  protected:

  //! @brief Throw unless every histogram of this group has a compatible counterpart in other. @param other group @param source caller
  void requireSameHistograms(const HistogramFolder & other, const char * source) const;
  //! @brief Histogram of other with the name of h, checked compatible. @param other group @param h histogram @param source caller
  static const Histogram & counterpart(const HistogramFolder & other, const Histogram & h, const char * source);

  };

  //! @relates HistogramFolder
  //! @brief Sum of two groups, histogram by histogram. @param lhs first @param rhs second @return lhs+rhs
  HistogramFolder operator+(const HistogramFolder & lhs, const HistogramFolder & rhs);
  //! @relates HistogramFolder
  //! @brief Difference of two groups, histogram by histogram. @param lhs first @param rhs second @return lhs-rhs
  HistogramFolder operator-(const HistogramFolder & lhs, const HistogramFolder & rhs);

} // namespace CAP

#endif /* CAP__HistogramFolder */
