use std::sync::{mpsc, Arc, Mutex};
use std::thread;
use std::thread::JoinHandle;

use crate::io::{EngineResponse, GuiActionRequest};
use crate::search::search;

pub fn engine(
    gui_rx: mpsc::Receiver<GuiActionRequest>,
    engine_tx: mpsc::Sender<EngineResponse>,
) -> Result<(), mpsc::RecvError> {
    let mut board = None;
    let stop = Arc::new(Mutex::new(false));
    let mut search_thread = None;

    // We loop forever, and only break out if the gui_rx channel is terminated.
    loop {
        match gui_rx.recv()? {
            GuiActionRequest::AreYouReady => {
                // Make sure we are ready, then say so.
                engine_tx.send(EngineResponse::IamReady).unwrap();
            }
            GuiActionRequest::SetPosition(b) => {
                board = Some(b);
                // Potentially do other required initialization.
            }
            GuiActionRequest::StartSearching(uci_time_control, uci_search_control) => {
                // Ensure the stopping marker is not set.
                {
                    let mut stop = stop.lock().unwrap();
                    *stop = false;
                }

                // Dispatch a search in a new thread. We store the handle to join with it when we stop the search.
                let stop = stop.clone();
                let engine_tx = engine_tx.clone();
                search_thread = Some(thread::spawn(move || {
                    search(
                        board.expect(
                            "Can't start searching without a position being specified first!",
                        ),
                        uci_time_control,
                        uci_search_control,
                        engine_tx,
                        stop,
                    )
                }));
            }
            GuiActionRequest::StopSearching => {
                // Set "stop" variable to true, then join with the search thread.
                // The search thread should ensure to output bestmove
                {
                    let mut stop = stop.lock().unwrap();
                    *stop = true;
                }

                // If a stop was sent without a running search, just ignore the command.
                search_thread.take().map(JoinHandle::join);
            }
        }
    }
}
