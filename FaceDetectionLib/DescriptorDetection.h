/**
* \file DescriptorDetection.h
* \brief This file defines class DescriptorDetection which has a couple functions to find and match descriptors on the picture.
*
*/

#pragma once

/** \class DescriptorDetection
* \brief This class includes two functions which work on with descriptors.
*
*/

class DescriptorDetection
{
public:

	/**
	* \brief Short description of function.
	*
	* Description of this function. It may have got some links to other function.
	* Sample: here is link to \a matchDescriptors
	*
	* \param[out]	face		target face
	* \param[in]	name		description of name.
	* \param[in]	showImage	found descriptors will be shown if bariable is set true
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	* code example
	*
	* \endcode
	*/

	Mat findDescriptors(Mat &face, char* name, bool b);

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

