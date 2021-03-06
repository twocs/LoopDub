// Program.cpp: implementation of the ProgramChanger class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include "program.h"
#include "settings.h"

#define programsfile "programs.ini"

/////////////////////////////////
// Program

Program::Program()
{
	 int i;
	 for (i=0; i<N_LOOPS; i++)
		  m_strFile[i][0] = 0;
}

/////////////////////////////////
// ProgramChanger

ProgramChanger::ProgramChanger()
{
	 m_Program = NULL;
	 m_hThread = 0;
	 m_nLoadingQueueW = m_nLoadingQueueR = 0;
}

ProgramChanger::~ProgramChanger()
{
	 if (m_Program)
		  delete[] m_Program;
}

/*!
** Load programs from programs.ini
*/
void ProgramChanger::LoadPrograms()
{
	 char line[1024], param[256], value[MAX_PATH];
	 int len, pos, mark, n;
	 int number=1, loop=0;

	 // count number of programs

     SettingsFile f(programsfile);
     if (!f.IsOpenForRead()) {
         printf("No programs found.\n");
         return;
     }
     
     m_nPrograms = 0;
     while (f.ReadSetting())
        if (f.m_bSectionChanged) m_nPrograms++;
     f.Close();

	 printf("%d program%s found.\n", m_nPrograms, m_nPrograms==1 ? "" : "s");
	 
	 // allocate
	 m_Program = new Program[m_nPrograms];

	 // rewind
     if (!f.OpenForRead(programsfile)) {
        printf("Error re-opening %s\n", programsfile);
        return;
     }

    n=-1;
    while (f.ReadSetting())
    {
        if (f.m_bSectionChanged) {
            n++;
			m_Program[n].m_nProgramNumber = n+1;
            strncpy(m_Program[n].m_strName, f.m_strSection, Program::m_nNameSize);
            loop = 1;
        }
        
		if (strcasecmp(f.m_strParam, "Dir")==0 && n>=0 && n<m_nPrograms) {
		    strncpy(m_Program[n].m_strDir, f.m_strValue, Program::m_nDirSize);
		}
	    else if (strcasecmp(f.m_strParam, "Loop")==0 && n>=0 && n<m_nPrograms) {
			pos = loop++;
			if (pos >= 1 && pos <= N_LOOPS)
				 strncpy(m_Program[n].m_strFile[pos-1], f.m_strValue, Program::m_nFileNameSize);
		}
    }
     
	 printf("%d program%s loaded.\n", m_nPrograms, m_nPrograms==1 ? "" : "s");
}

/*!
 * Load the samples in the queue in order, and drop them into their corresponding LoopOb instances as
 * "background samples" once they are loaded.
 */

THREADFUNC ProgramChanger::loadBackgroundSamplesThread(void* param)
{
#ifndef WIN32
	 pthread_detach(pthread_self());
#endif
	 LOWPRIORITY();

	 ProgramChanger &pc = *((ProgramChanger*)param);

	 while (pc.m_nLoadingQueueR != pc.m_nLoadingQueueW) {
		  char *strFilename = (char*)pc.m_pLoadingQueue[pc.m_nLoadingQueueR++];
		  LoopOb *pLoopOb = (LoopOb*)pc.m_pLoadingQueue[pc.m_nLoadingQueueR++];
		  pc.m_nLoadingQueueR = pc.m_nLoadingQueueR % 20;

		  // Skip it if it has the same filename as foreground sample or previous background sample
		  if (pLoopOb->GetSample() && strcmp(pLoopOb->GetSample()->m_filename, strFilename)==0)
			   continue;
		  if (pLoopOb->GetBackgroundSample() && strcmp(pLoopOb->GetBackgroundSample()->m_filename, strFilename)==0)
			   continue;
		  
		  Sample *pSample = new Sample();
		  if (pSample->LoadFromFile(strFilename)) {
			   pLoopOb->SetBackgroundSample(pSample);
		  }
		  else
			   printf("Error loading %s as background sample.\n", strFilename);
	 }

	 pc.m_hThread = 0;
}

/*!
** Setup m_pLoopOb array to switch to new program on zero volume.
*/
void ProgramChanger::ProgramChange(int program, LoopOb* m_pLoopOb[N_LOOPS])
{
	 // Find the program
	 int i;
	 for (i=0; i<m_nPrograms; i++)
		  if (m_Program[i].m_nProgramNumber == program) break;
	 if (i>=m_nPrograms) {
         printf("Program %d not found.\n", program);
         return;
     }

	 // If we are already loading something, clear the queue.
	 m_nLoadingQueueR = m_nLoadingQueueW;

	 // Put each sample filename onto the queue along with its
	 // associated LoopOb object.
	 int loop;
	 for (loop=0; loop<N_LOOPS; loop++)
	 {
		  if (m_Program[i].m_strFile[loop][0]) {
			   m_pLoadingQueue[m_nLoadingQueueW] = m_Program[i].m_strFile[loop];
			   m_pLoadingQueue[m_nLoadingQueueW+1] = m_pLoopOb[loop];
			   m_nLoadingQueueW = (m_nLoadingQueueW+2) % 20;
		  }
		  if (m_Program[i].m_strDir[0]) {
			   m_pLoopOb[loop]->m_pFileBrowser->SetDirectoryFromBase(m_Program[i].m_strDir);
		  }
	 }

	 printf("Changing to program %d.\n", program);
	 if (!m_hThread)
		  CREATETHREAD(m_hThread, loadBackgroundSamplesThread, (void*)this);
}
