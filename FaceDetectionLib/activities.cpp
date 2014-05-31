#include "stdafx.h"
#include "LibInclude.h"
#include "io.h"
#include <ctime>

FaceCascades cascades[MAX_THREADS_AND_CASCADES_NUM];

DWORD getFacesFromPhoto(void *pContext)
{
	double startTime = clock();
	ContextForGetFaces *psContext = (ContextForGetFaces*)pContext;

	string photoName = ((string)ID_PATH).append(psContext->userId).append("\\photos\\").append(psContext->photoName).append(".jpg");
	
	IplImage *img = cvLoadImage(photoName.c_str());
	
	if (img == NULL)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s. Continue..."), photoName.c_str());
		net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));
		delete psContext;
		return -1;
	}

	ViolaJonesDetection detector(cascades);

	try{
		if (!detector.allFacesDetection(img, psContext->sock))
		{
			FilePrintMessage(NULL, _FAIL("All Faces Detection FAILED"), photoName.c_str());
			net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));
			delete psContext;
			return -1;
		}
	}
	catch (...)
	{
		FilePrintMessage(NULL, _FAIL("All Faces Detection FAILED"), photoName.c_str());
		net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));
		delete psContext;
		return -1;
	}

	net.SendData(psContext->sock, "{ \"success\":\"get faces succeed\" }\n\0", strlen("{ \"success\":\"get faces succeed\" }\n\0"));

	FilePrintMessage(NULL, _SUCC("Get faces finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	delete psContext;
	return 0;
}

DWORD saveFaceFromPhoto(void *pContext)
{
	double startTime = clock();
	ContextForSaveFaces *psContext = (ContextForSaveFaces*)pContext;

	string photoName = ((string)ID_PATH).append(psContext->userId).append("\\photos\\").append(psContext->photoName).append(".jpg");

	IplImage *img = cvLoadImage(photoName.c_str());

	if (img == NULL)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s. Continue..."), photoName.c_str());
		net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));
		delete psContext;
		return -1;
	}
	//необходимо подавать черно-белую картинку в cutFaceToBase
	ViolaJonesDetection detector(cascades);

	try
	{
		if (!detector.cutTheFace(img, ((string)ID_PATH).append(psContext->userId).append("\\faces\\").append(psContext->photoName).append(".jpg").c_str(), psContext->faceNumber))
		{
			FilePrintMessage(NULL, _FAIL("cut face FAILED"), photoName.c_str());
			net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));
			delete psContext;
			return -1;
		}
	}
	catch (...)
	{
		FilePrintMessage(NULL, _FAIL("cut face FAILED"), photoName.c_str());
		net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));
		delete psContext;
		return -1;
	}

	net.SendData(psContext->sock, "{ \"success\":\"cut face succeed\" }\n\0", strlen("{ \"success\":\"cut face succeed\" }\n\0"));

	FilePrintMessage(NULL, _SUCC("Cut face finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	delete psContext;
	return 0;
}

DWORD recognizeFromModel(void *pContext)
{
	double startTime = clock();
	ContextForRecognize *psContext = (ContextForRecognize*)pContext;
	//CvMemStorage* storage = NULL;
	IplImage *img = NULL;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection(cascades);
	map <string, Ptr<FaceRecognizer>> models;

	for (UINT_PTR i = 0; i < psContext->arrFrinedsList.size(); i++)
	{
		Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
		try
		{
			model->load(((string)(ID_PATH)).append(psContext->arrFrinedsList[i].operator std::string()).append("//eigenface.yml"));
		}
		catch (...)
		{
			FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), psContext->arrFrinedsList[i].ToString().c_str());
			continue;
		}
		models[psContext->arrFrinedsList[i].ToString()] = model;
	}

	if (models.empty())
	{
		FilePrintMessage(NULL, _FAIL("No models loaded."));
		net.SendData(psContext->sock, "{ \"error\":\"training was not called\" }\n\0", strlen("{ \"error\":\"training was not called\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}

	try
	{
		img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg.append(".jpg")).c_str());
	}
	catch (...)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
		net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	

	if (!img)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
		net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}

	//storage = cvCreateMemStorage();					// Создание хранилища памяти
	try
	{
		if (!violaJonesDetection->faceDetect(img, models, psContext->good_id, psContext->sock))
		{
			FilePrintMessage(NULL, _FAIL("Some error occured during recognze call"));
			net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
			delete violaJonesDetection;
			delete psContext;
			return -1;
		}
	}
	catch (...)
	{
		FilePrintMessage(NULL, _FAIL("Some error occured during recognze call"));
		net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}

#ifdef SHOW_IMAGE
	while (1){
		if (cvWaitKey(0) == 27)
			break;
	}
#endif

	net.SendData(psContext->sock, "{ \"success\":\"recognize faces succeed\" }\n\0", strlen("{ \"success\":\"recognize faces succeed\" }\n\0"));

	FilePrintMessage(NULL, _SUCC("Recognize finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	cvReleaseImage(&img);
	//cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	delete psContext;
	return 0;
}

DWORD generateAndTrainBase(void *pContext)
{
	double startTime = clock();

	ContextForTrain *psContext = (ContextForTrain*)pContext;
	_finddata_t result;
	HANDLE *phEventTaskCompleted = new HANDLE[psContext->arrIds.size()];
	std::vector <HANDLE> threads;
	UINT_PTR uSuccCounter = 0;

	for (UINT_PTR i = 0; i < psContext->arrIds.size(); i++)
	{
		memset(&result, 0, sizeof(result));
		string photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\*.jpg");

		intptr_t firstHandle = _findfirst(photoName.c_str(), &result);

		UINT_PTR uNumOfThreads = 0;

		if (firstHandle != -1)
		{
			do
			{
				photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\").append(result.name);
				IplImage *img = cvLoadImage(photoName.c_str());
				if (img == NULL)
				{
					FilePrintMessage(NULL, _WARN("Failed to load image %s. Continue..."), photoName.c_str());
					continue;
				}

				cutFaceThreadParams * param = new cutFaceThreadParams(img,
					(((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\faces\\").append(result.name)).c_str(),
					&cascades[uNumOfThreads]);
				threads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)param->pThis->cutFaceThread, (LPVOID)param, 0, NULL));

				if (++uNumOfThreads == MAX_THREADS_AND_CASCADES_NUM)
				{
					DWORD res;
					if (WAIT_OBJECT_0 != (res = WaitForMultipleObjects((unsigned)threads.size(), &threads[0], TRUE, CUT_TIMEOUT)))
					{
						FilePrintMessage(NULL, _FAIL("Timeout has occured during waiting for cutting images finished"));
						for (UINT_PTR j = 0; j < threads.size(); j++)
						{
							TerminateThread(threads.at(j), -1);
						}
					}

					for (UINT_PTR j = 0; j < threads.size(); j++)
					{
						CloseHandle(threads.at(j));
					}
					threads.clear();

					uNumOfThreads = 0;
				}

			} while (_findnext(firstHandle, &result) == 0);

			if (uNumOfThreads != 0)
			{
				DWORD res;
				if (WAIT_OBJECT_0 != (res = WaitForMultipleObjects((unsigned)threads.size(), &threads[0], TRUE, CUT_TIMEOUT)))
				{
					FilePrintMessage(NULL, _FAIL("Timeout has occured during waiting for cutting images finished"));
					for (UINT_PTR j = 0; j < threads.size(); j++)
					{
						TerminateThread(threads.at(j), -1);
					}
				}

				for (UINT_PTR j = 0; j < threads.size(); j++)
				{
					CloseHandle(threads.at(j));
				}
				threads.clear();

				uNumOfThreads = 0;
			}
		}

		EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

		//train FaceRecognizer
		try
		{
			if (!eigenDetector_v2->train((((string)ID_PATH).append(psContext->arrIds[i].ToString())).c_str()))
			{
				delete eigenDetector_v2;
				continue;
			}
		}
		catch (...)
		{
			FilePrintMessage(NULL, _FAIL("Some error has occured during Learn call."));
			delete eigenDetector_v2;
			continue;
		}
		delete eigenDetector_v2;
		uSuccCounter++;
	}

	cvDestroyAllWindows();
	FilePrintMessage(NULL, _SUCC("Train finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	if (uSuccCounter == 0)
	{
		net.SendData(psContext->sock, "{ \"fail\":\"learning failed\" }\n\0", strlen("{ \"fail\":\"learning failed\" }\n\0"));
		return -1;
	}

	net.SendData(psContext->sock, "{ \"success\":\"train succeed\" }\n\0", strlen("{ \"success\":\"train succeed\" }\n\0"));
	return 0;
}