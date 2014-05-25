/**
* \file DescriptorDetection.h
* \brief This file defines class DescriptorDetection which has a couple functions to find and match descriptors on the picture.
*
*/

#pragma once


/** \addtogroup DescriptorDetection
*  \brief Descriptor detection subsystem:
*  \{
*/

/** \class DescriptorDetection
* \brief This class includes two functions which work on with descriptors.
*
*/

class DescriptorDetection
{
public:

	/**
	* \brief This function defines descriptor.
	*
	* Description of this function. It may have got some links to other function.
	* Sample: here is link to \a matchDescriptors
	*
	*\return number of matches. (Warinng! Valid number of matches depends on size of target picture!)
	*
	* \param[in]	face		target face
	* \param[in]	name		description of name.
	* \param[in]	oShowImage	found descriptors will be shown if bariable is set true
	*
	*/

	Mat findDescriptors(Mat &face, char* name, bool oShowImage);

	/**
	* \brief getting match for 2 descriptors.
	*
	* You can find specified object (for example eye) using this function.
	* You can use \a findDescriptors function to find faces' descriptors on image
	*
	* \return number of matches. (Warinng! Valid number of matches depends on size of target picture!)
	*
	* \param[in] &ffd			(FoundFaceDescriptors)descriptor of current image's frame
	* \param[in] &bfd			(BaseFaceDescriptors) descriptor of image you want to find
	*
	*/

	int matchDescriptors(Mat &ffd, Mat &bfd);
};

/** \} */

