##########################################################################################################################
# Writer Retrieval System
# Bag of feature implementation of Writer retrieval system using SIFT
##########################################################################################################################	

Deploy: 
- To run the application, double click the AuthorIdentificationNew application file in build directory.

##########################################################################################################################
Build: (Optional)
- The include and lib path are mentioned in the .pro file.
- Dependency: opencv 3.1.0
			opencv_contrib_master (for non-free components)
			openmp
			qt
- c++11 functionality is used.

#########################################################################################################################
Code Structure:
The application has three classes -
- MainWindow for training vocabulary and histogram database creation.
- TestWindow is for testing single or all query images. 
- KeypointDetection is used for tuning keypoint detection parameters.

#########################################################################################################################
Functionality:
Training:
1. In Training window, select training images by clicking on the "Browse to Upload" push button. 
   This extracts features and generates a vocabulary file named dictionary_600.yml in the out directory for codebook size of 600. 
2. Once the vocabulary is created, select the same by clicking on the "Browse to Upload" push button in the create histogram database section. 
3. Click on "Create Histogram Database" button to generate histogram for all training images loaded from the training directory path.

Testing:
1. In Test window, select the vocabulary file by clicking on the "Upload Vocabulary" button. 
2. Select the histogram database file by clicking on the "Upload histogram database" button. 
3. Either select one query image by "Upload query image". This displays the query image and the topmost match in new windows. 
   This method also shows the top 5 match and the corresponding distance from the query image. In addition to this, it also verifies the
   program output with actual output and reports the success rate and recall rate.
   Note: Please rerun to test for another query image.
4. Alternatively to step 3, all test data from the test directory path can also be tested by clicking the "Calculate test statistics for All Test Data".
   This method generates two files top5_600_CT_cosinesim.txt and top5_600_CT_chisquare.txt in the out directory. 
   These files report the overall success and recall rate and also provides details at test data level.

##########################################################################################################################
User's Guide:
PRE_REQUISITE:
1. Place the training images in the training directory path as described by the variable training_path in main.cpp. Remove all hidden files if exist.
2. Place the test images in the test directory path as described by the variable test_path in main.cpp. Remove all hidden files if exist.
3. Create the out directory as described by the variable out_dir in main.cpp.
4. Ensure that forms1.txt, uniq_writer_img_list.txt and single_sample_list.txt exsist in the path, described by the variables
   form_file_path,uniqWriterImgFile_path and single_sample_list in main.cpp . 
5. Keep the dictionary file and the database file ready.
6. To run the keypointdetection, uncomment line number 38 in main.cpp. 
   This will extract keypoints on the images residing at the path described by variable sample_imgs_path in main.cpp for different threshold and sigma values.
   In the end it will display the outputs and also save them in out directory.
