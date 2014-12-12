**NovaSearchServices** is a set of APIs that allows the creation of
applications that feature extraction for images and text, indexing and
retrieval.

Build on top of the best available image processing, classification and
retrieval libraries, It was designed for speed (programmed in C++) and
easy of use for rapid prototyping using state-of-the-art image
processing, indexing and machine learning algorithms

The repo for the framework is available [here][] (request access to
André Mourão).

Details for the framework components:
-------------------------------------

### Core

#### Analyser

Using a factory paradigm and common interfaces, the framework enables
simple interchanging of components. These features make testing multiple
combinations of techniques painless and allow rapid prototyping and the
creation of perfect solutions for a wide range of CBMI and
classification tasks and challenges.

**Available algorithms**

**Feature extractors:**

-   Global image features (nVector/FeatureExtractor.h)
    -   Gabor
    -   GIST
    -   LBP
    -   HSV histograms
    -   CEDD
    -   FCTH
    -   Eigen extractor (PCA)
-   Keypoint based (nKeypoint/KeypointFeatureExtractor.h)
    -   SURF
    -   SIFT
    -   ORB

**Object detectors** (nRoi/RoiFeatureExtractor.h):

-   Face detection
-   Person detection
-   Generic Haar-based object detector

**Classifiers** (nTag/IClassifier.h):

-   Sparse approximation
-   SVM
-   Vowpal Wabbit
-   kNN

#### Indexing

The indexers follow the same interchangeability model. Available
implementations allow balancing speed and accuracy to answer any task
requirements.

**Indexers:**

-   Sparse approximation
-   MSIDX
-   E2LSH
-   Spherical Hashing
-   Linear
-   kd and kMeans tree

#### Endpoints

NSS runs as a service that communicates using REST endpoints Developers
can hand-craft solution to the retrieval or classification tasks, or use
our generic classifier or indexer that leverage on the
interchangeability properties of the framework.

### Extra

#### Multimodal retrieval support

The framework supports retrieval from independent indexes that
communicate through REST and output JSON in a simple standard format.
The results are combined using a Federator, a Django-based application
that makes the requests to the indexes, combines and outputs the final
rank. It is available [here][1] (request access to André Mourão).

#### External feature extractors

The framework supports feature extractors from “outside” programs
through REST and output JSON in a simple standard format.

Setup
-----

### Dependencies:

-   HDF5
-   Flann
-   OpenCV 2.4.9
-   FFTW 3
-   Armadillo
-   lapack
-   blas
-   Poco
-   libcurl
-   pugixml
-   jsoncpp

These libraries are available at Ariadne under
`/localstore/searchservices/myLibs/`.

Before compiling and running export the following libraries:

`export LD_LIBRARY_PATH=/localstore/searchservices/myLibs/lib:$LD_LIBRARY_PATH && export LIBRARY_PATH=/localstore/searchservices/myLibs/bin:$LIBRARY_PATH && export C_INCLUDE_PATH=/localstore/searchservices/myLibs/include:$C_INCLUDE_PATH && export CPLUS_INCLUDE_PATH=/localstore/searchservices/myLibs/include:$CPLUS_INCLUDE_PATH && export LD_LIBRARY_PATH=/usr/lib/jvm/java-7-oracle/:$LD_LIBRARY_PATH && export LD_LIBRARY_PATH=/usr/lib/jvm/java-7-oracle/jre/lib/amd64/server/:$LD_LIBRARY_PATH`

  [here]: https://bitbucket.org/a_mourao/searchservices
  [1]: https://bitbucket.org/a_mourao/federator