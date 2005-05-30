


#define _LOOPDUB_CPP_
#include <stdio.h>
#include <string.h>
#include "loopdub.h"
#include "ld_logo.h"

#ifdef WIN32
#else
  #include <unistd.h>
#endif

/* One global LoopDub object */
LoopDub app;

/*
** LoopDub
*/

#define APP_VERSION "0.1"
#define LOOPTOP    50
#define LOOPHEIGHT 60

LoopDub::LoopDub()
{
	 m_strChangeToFolder = NULL;
	 m_nBeats = 4;
	 m_nPos = 0;
	 m_nLength = 0;
	 m_pMidiLearning = NULL;
	 m_nKeysChannel = -1;
	 for (int i=0; i<MAX_KEYS; i++)
		  m_Keys[i].on = false;

	 CREATEMUTEX(mutex);
}

LoopDub::~LoopDub()
{
	DESTROYMUTEX(mutex);
}

bool priority = false;
void LoopDub::FillBuffers(void *param)
{
	LoopDub& app = *(LoopDub*)param;

	if (!priority) {
		 HIGHPRIORITY();
		 priority = true;
	}

	LOCKMUTEX(app.mutex);

	short* pBufferL = app.m_Player.LeftBuffer();
	short* pBufferR = app.m_Player.RightBuffer();
	int n=app.m_Player.BufferSizeSamples();
	int i, max=0;
	int volume, volmax;
	
	volume = app.m_pVolumeSlider->GetValue();
	volmax = app.m_pVolumeSlider->GetMaxValue();

	while (n-- > 0)
	{
		 int value[2];
		 int side=0;
		 value[0] = value[1] = 0;
		*pBufferR = *pBufferL = 0;
		for (i=0; i<N_LOOPS; i++)
		{
			 side = app.m_pLoopOb[i]->IsCue() ? 1 : 0;
			 if (i!=app.m_nKeysChannel) {
				  value[side] += app.m_pLoopOb[i]->GetSampleValue(app.m_nPos);
			 }
			 else {
				  for (int key=0; key <MAX_KEYS; key++) {
					   if (app.m_Keys[key].on) {
							value[side] += app.m_pLoopOb[i]->GetSampleValue(app.m_Keys[key].position >> 10);
							app.m_Keys[key].position += app.m_Keys[key].velocity;
							if ((app.m_Keys[key].position >> 10) > (app.m_pLoopOb[i]->GetSample()->m_nSamples))
								 app.m_Keys[key].on = false;
					   }
				  }
			 }
		}

		for (int i=0; i<2; i++) {
			 value[i] = value[i] * volume / volmax;

			 if (value[i] > 32767)  value[i] = 32767;
			 if (value[i] < -32767) value[i] = -32767;
			 if (value[i] > max) max = value[i];
		}

		*(pBufferR++) = value[0];
		*(pBufferL++) = value[1];

		if (++app.m_nPos > app.m_nLength)
			 app.m_nPos = 0;
	}

	app.m_pVUMeter->SetPercentage(max * 100 / 32767);

	if (app.updated) {
		 SDL_Event evt;
		 evt.type = SDL_USEREVENT;
		 SDL_PushEvent(&evt);
		 app.updated = false;
	}

	UNLOCKMUTEX(app.mutex);
}

THREADFUNC loadSampleThread(void* pApp)
{
#ifndef WIN32
	 pthread_detach(pthread_self());
#endif

	 LOWPRIORITY();

	if (!pApp) return NULL;

	 LoopDub &app = *(LoopDub*)pApp;

	 Sample *pSample = new Sample();
	 if (pSample && pSample->LoadFromFile(app.m_strLoadingSample))
	 {
		  LOCKMUTEX(app.mutex);
		  if (app.m_nLength==0) app.m_nLength = pSample->m_nSamples;
		  Sample *pOldSample = app.m_pLoopOb[app.m_nLoadingSampleFor]->SetSample(pSample);
		  if (pOldSample)
			   delete pOldSample;
		  UNLOCKMUTEX(app.mutex);
	 }

	 return NULL;
}

int LoopDub::Run()
{
	printf("LoopDub started...\n");

	if (m_Midi.Initialize())
		 printf("MIDI initialized.\n");
	else
		 printf("Couldn't initialize MIDI.\n");

	if (!gui.Initialize("LoopDub", false, 800, 600, SDL_INIT_TIMER))
	{
		printf("Error initializing GUI.\n");
		return 1;
	}

	Scrob *pMainScrob = gui.GetScrobList();

	/* Change to loop folder */
	if (m_strChangeToFolder && chdir(m_strChangeToFolder))
		 printf("Warning: Couldn't change to %s\n", m_strChangeToFolder);

	/* LoopOb instances */
	for (int i=0; i<N_LOOPS; i++)
	{
		m_pLoopOb[i] = new LoopOb(pMainScrob,
								  Rect(5,
									   LOOPTOP+(LOOPHEIGHT+5)*i,
									   pMainScrob->GetRect().Width() - 5,
									   LOOPHEIGHT+LOOPTOP+(LOOPHEIGHT+5)*i),
								  i);
		pMainScrob->AddChild(m_pLoopOb[i]);
	}
	m_pLoopOb[3]->SetSelected(true);

	/* Controls along the top */

	Button *pBeats = new Button(pMainScrob, Rect(5, 5, 55, 20), "beats", 0, 2, CMD_BEATS);
	pMainScrob->AddChild(pBeats);

	m_pAutoCueButton = new Button(pMainScrob, Rect(5, 22, 55, 37), "AutoCue", 0, 2, -1, 0, true);
	pMainScrob->AddChild(m_pAutoCueButton);

	Label *pTempo = new Label(pMainScrob, Rect(60, 5, 120, 20), NULL, 3, 0);
	pMainScrob->AddChild(pTempo);

	m_pVUMeter = new VUMeter(pMainScrob, Rect(130, 5, 230, 20));
	pMainScrob->AddChild(m_pVUMeter);

	m_pVolumeSlider = new Slider(pMainScrob, Rect(130,27,230,40), false);
	pMainScrob->AddChild(m_pVolumeSlider);
	m_pVolumeSlider->SetValue(m_pVolumeSlider->GetMaxValue() / 2);

	pMainScrob->AddChild(
		 new Image(pMainScrob, Rect(780-logo_width, 10, 780, 10+logo_height),
				   logo_width, logo_height, logo_data)
		 );

/*
	m_pTempoSlider = new Slider(pMainScrob, Rect(5, 5, 205, 20), false);
	pMainScrob->AddChild(m_pTempoSlider);
	m_pTempoSlider->SetValue((m_nTempo-MIN_TEMPO)*m_pTempoSlider->GetMaxValue()/(MAX_TEMPO-MIN_TEMPO));

	Label *pLabel = new Label(pMainScrob, Rect(210, 5, 250, 20), "Tempo", 3, 0);
	pMainScrob->AddChild(pLabel);
	pLabel->SetInteger(100);
*/

	/* Midi controls */

	if (m_Midi.IsInitialized())
	{
		 pMainScrob->AddChild(new Label(pMainScrob, Rect(240, 5, 290, 20), "Midi:", 3, 0));

		 m_pMidiLearning = new Button(pMainScrob, Rect(290, 5, 340, 20), "Learn", 0, 2, CMD_LEARN, 0, true);
		 pMainScrob->AddChild(m_pMidiLearning);

		 int n = m_Midi.GetMidiNum();
		 for (int i=0; i<n; i++)
		 {
			  pMainScrob->AddChild(new Button(pMainScrob, Rect(350 + 50*i, 5, 400 + 50*i, 20),
											  m_Midi.GetMidiName(i), 0, 2, CMD_SELECT, (void*)i, true));
		 }
	}

	/* Load program changer */
	m_ProgramChanger.LoadPrograms();

	/* Initialize player */

	if (!m_Player.Initialize(FillBuffers, this))
	{
		printf("Couldn't initialize player.\n");
		return 1;
	}

	m_Player.Play();

	SDL_TimerID timerID=0;
	SDL_Event *pEvent;
	bool bQuit=false;
	int i;
	while (!bQuit)
	{
		gui.WaitEvent();

		LOCKMUTEX(mutex);

		if ((pEvent=gui.GetEvent())->type == SDL_KEYDOWN
			&& pEvent->key.keysym.sym == SDLK_RETURN
			&& pEvent->key.keysym.mod == KMOD_NONE)
		{
			if (m_Player.IsPlaying())
				m_Player.Stop();
			else
				m_Player.Play();
		}
		else if ((pEvent=gui.GetEvent())->type == SDL_KEYDOWN
			&& pEvent->key.keysym.sym == SDLK_SPACE
			&& pEvent->key.keysym.mod == KMOD_NONE)
		{
			 // find unused key
			 for (i=0; i<MAX_KEYS && m_Keys[i].on; i++);
			 if (i<MAX_KEYS) {
				  m_Keys[i].position = 0;
				  m_Keys[i].velocity = 1024;
				  m_Keys[i].note = 48; // default key C4
				  m_Keys[i].on = true;
			 }
		}
//		else if ((pEvent=gui.GetEvent())->type == SDL_KEYDOWN
//				 && pEvent->key.keysym.sym >= '1') {
//			 m_ProgramChanger.ProgramChange(1, m_pLoopOb);
//		}
		else if ((pEvent=gui.GetEvent())->type == SDL_KEYDOWN
			&& pEvent->key.keysym.sym >= '1'
			&& pEvent->key.keysym.sym <= '8'
			&& pEvent->key.keysym.mod == KMOD_NONE)
		{
			 int ch = pEvent->key.keysym.sym - '1';
			 m_pLoopOb[ch]->SetSelected(!m_pLoopOb[ch]->IsSelected());
		}
		else
			bQuit = !gui.ProcessEvent();

		if (!bQuit)
		{
			 int cmd;
			 void *value;
			 if (gui.GetCommand(&cmd, &value))
			 {
				  if (cmd==CMD_LEARN)
				  {
					   if (m_pMidiLearning)
							m_Midi.SetLearningMode(m_pMidiLearning->IsPressed());
				  }
				  else if (cmd==CMD_SELECT)
				  {
					   printf("Select: %d\n", (int)value);
					   m_Midi.SelectDevice((int)value);
				  }
				  else if (cmd==CMD_BEATS)
				  {
					   m_nBeats = (m_nBeats*2)%128;
					   if (m_nBeats<=0) m_nBeats=1;
					   char str[10];
					   sprintf(str, "%d beat%s", m_nBeats, (m_nBeats>1)?"s":"");
					   pBeats->SetText(str);
					   if (m_pLoopOb[0]->GetSample()) {
							int length = m_pLoopOb[0]->GetSample()->m_nSamples;
							int bpm = SAMPLE_RATE*m_nBeats*60/length;
							//	(samples/sec) / (samples/beats) = (samples/sec) * (beats/sample) = (beats/sec) * (sec/min) = (beats/min)
							sprintf(str, "%d bpm", bpm);
							pTempo->SetText(str);
					   }
				  }
				  else if (cmd>=CMD_FILECLICK && cmd<(CMD_FILECLICK+N_LOOPS))
				  {
					   int s = cmd-CMD_FILECLICK;

					   m_nLoadingSampleFor = s;
					   strcpy(m_strLoadingSample, (char*)value);

					   HTHREAD thread;
					   CREATETHREAD(thread, loadSampleThread, &app);
				  }
				  else if (cmd==CMD_DIRCLICK)
				  {
					   // Set all filebrowsers to same folder
					   for (int ch=0; ch<N_LOOPS; ch++)
							m_pLoopOb[ch]->m_pFileBrowser->SetDirectory((char*)value);
				  }
				  else if (cmd>=CMD_CLOSE && cmd<(CMD_CLOSE+N_LOOPS))
				  {
					   int s = cmd-CMD_CLOSE;
					   Sample *pOldSample = m_pLoopOb[s]->SetSample(NULL);
					   if (pOldSample)
							delete pOldSample;
				  }
				  else if (cmd==CMD_NORMALIZE) {
					   int ch = (int)value;
					   if (m_pLoopOb[ch]->GetSample()) {
							m_pLoopOb[ch]->GetSample()->Normalize();
							m_pLoopOb[ch]->SetSample(m_pLoopOb[ch]->GetSample());
							m_pLoopOb[ch]->SetDirty();
					   }
				  }
				  else if (cmd==CMD_KEYS) {
					   int ch = (int)value;
					   if (m_nKeysChannel > -1)
							m_pLoopOb[m_nKeysChannel]->LoseKeys();

					   if (m_pLoopOb[ch]->HasKeys() && !m_pLoopOb[ch]->GetSample())
							m_pLoopOb[ch]->LoseKeys();

					   if (m_pLoopOb[ch]->HasKeys())
							m_nKeysChannel = ch;
					   else
							m_nKeysChannel = -1;
				  }
				  else if (cmd==CMD_SPLIT) {
						int ch = (int)value;
						m_pLoopOb[ch]->Split();
				  }
				  else if (cmd==CMD_HOLD) {
						int ch = (int)value;
						m_pLoopOb[ch]->SetHolding(m_pLoopOb[ch]->m_pHoldButton->IsPressed());
				  }
				  else if (cmd==CMD_PROGRAMCHANGE) {
					   m_ProgramChanger.ProgramChange((int)value, m_pLoopOb);
				  }
			 }

			 app.updated = true;

			 UNLOCKMUTEX(mutex);
			 m_Midi.CheckMsg();

			 // Check if its time to switch to background sample.
			 for (i=0; i<N_LOOPS; i++)
				  m_pLoopOb[i]->CheckBackgroundSample();
		}
	}

	app.updated = true;
	UNLOCKMUTEX(mutex);
	m_Player.Stop();

	return 0;
}

int main(int argc, char* argv[])
{
	 if ((argc > 1) && argv[1]) {
		  if (strcmp(argv[1], "-v")==0) {
			   printf("LoopDub " APP_VERSION "\n");
			   exit(0);
		  }
		  else if (strcmp(argv[1], "--help")==0) {
			   printf("Usage: loopdub <folder>\n");
			   exit(0);
		  }
		  app.m_strChangeToFolder = argv[1];
	 }

	return app.Run();
}




