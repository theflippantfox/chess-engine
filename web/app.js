var board = null;
var game = new Chess();
var playerColor = "w";
var engineLines = [];
var ws = null;
var engineReady = false;

var statusEl = $("#status");
var depthEl = $("#depth");
var scoreEl = $("#score");
var nodesEl = $("#nodes");

function setStatus(msg) {
  statusEl.text(msg);
}

function updateInfo(depth, score, nodes) {
  depthEl.text(depth ? "d" + depth : "");
  scoreEl.text(score != null ? (score / 100).toFixed(2) : "");
  nodesEl.text(nodes ? nodes + " nps" : "");
}

function connectEngine() {
  var protocol = location.protocol === "https:" ? "wss:" : "ws:";
  ws = new WebSocket(protocol + "//" + location.host);

  ws.onopen = function () {
    setStatus("Engine starting...");
    send("uci");
  };

  ws.onmessage = function (evt) {
    var msg = JSON.parse(evt.data);
    if (msg.type === "engine") {
      handleEngineLine(msg.line);
    }
  };

  ws.onclose = function () {
    setStatus("Disconnected. Reconnecting...");
    engineReady = false;
    setTimeout(connectEngine, 2000);
  };
}

function send(cmd) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: "uci", line: cmd }));
  }
}

function sendPosition() {
  var history = game.history({ verbose: true });
  if (history.length > 0) {
    var uciMoves = history.map(function (m) {
      return m.from + m.to + (m.promotion || "");
    });
    send("position startpos moves " + uciMoves.join(" "));
  } else {
    send("position startpos");
  }
}

function requestEngineMove() {
  setStatus("Thinking...");
  sendPosition();
  send("go depth 4");
}

function handleEngineLine(line) {
  if (line === "uciok") {
    send("isready");
    return;
  }
  if (line === "readyok") {
    engineReady = true;
    setStatus("Your turn — " + (playerColor === "w" ? "White" : "Black"));
    $("#newGameBtn, #flipBtn").prop("disabled", false);
    return;
  }
  if (line.startsWith("info")) {
    var depthMatch = line.match(/depth (\d+)/);
    var scoreMatch = line.match(/score cp (-?\d+)/);
    var nodesMatch = line.match(/nodes (\d+)/);
    if (depthMatch && scoreMatch) {
      var displayScore = playerColor === "w" ? parseInt(scoreMatch[1]) : -parseInt(scoreMatch[1]);
      updateInfo(depthMatch[1], displayScore, nodesMatch ? nodesMatch[1] : null);
    }
    return;
  }
  if (line.startsWith("bestmove")) {
    var parts = line.split(" ");
    if (parts.length < 2 || parts[1] === "(none)") {
      var result = game.fen();
      if (game.in_checkmate()) {
        var winner = game.turn() === "w" ? "Black" : "White";
        setStatus("Checkmate — " + winner + " wins!");
      } else if (game.in_stalemate()) {
        setStatus("Stalemate — draw");
      } else if (game.in_draw()) {
        setStatus("Draw");
      }
      return;
    }
    var moveStr = parts[1];
    var from = moveStr.substring(0, 2);
    var to = moveStr.substring(2, 4);
    var promo = moveStr.length > 4 ? moveStr[4] : undefined;

    var move = game.move({ from: from, to: to, promotion: promo });
    if (move) {
      board.position(game.fen(), true);
      highlightLastMove(move);
    }
    updateInfo(null, null, null);

    if (game.game_over()) {
      if (game.in_checkmate()) {
        var winner = game.turn() === "w" ? "Black" : "White";
        setStatus("Checkmate — " + winner + " wins!");
      } else if (game.in_stalemate()) {
        setStatus("Stalemate — draw");
      } else if (game.in_draw()) {
        setStatus("Draw");
      }
    } else {
      setStatus("Your turn");
    }
    return;
  }
}

function highlightLastMove(move) {
  $(".highlight-square").removeClass("highlight-square");
  $("#" + move.from).addClass("highlight-square");
  $("#" + move.to).addClass("highlight-square");
}

function onDragStart(source, piece) {
  if (game.game_over()) return false;
  if (!engineReady) return false;
  if (game.turn() !== playerColor) return false;
  if (piece.search(playerColor) === -1) return false;
}

function onDrop(source, target) {
  $(".highlight-square").removeClass("highlight-square");

  var move = game.move({
    from: source,
    to: target,
  });

  if (move === null) {
    var piece = game.get(source);
    if (piece && piece.type === "p") {
      var targetRank = target[1];
      if ((piece.color === "w" && targetRank === "8") ||
          (piece.color === "b" && targetRank === "1")) {
        showPromotionDialog(source, target);
        return "snapback";
      }
    }
    return "snapback";
  }

  highlightLastMove(move);

  if (game.game_over()) {
    if (game.in_checkmate()) {
      var winner = game.turn() === "w" ? "Black" : "White";
      setStatus("Checkmate — " + winner + " wins!");
    } else {
      setStatus("Game over — draw");
    }
    return;
  }

  requestEngineMove();
}

function showPromotionDialog(source, target) {
  var color = game.turn();
  var pieces = ["q", "r", "b", "n"];
  var names = { q: "Queen", r: "Rook", b: "Bishop", n: "Knight" };
  var dir = color === "w" ? 7 : 0;

  var overlay = $('<div id="promo-overlay"></div>');
  var dialog = $('<div id="promo-dialog"></div>');

  pieces.forEach(function (p) {
    var pieceImg = "https://chessboardjs.com/img/chesspieces/wikipedia/"
      + color + p.toUpperCase() + ".png";
    var btn = $('<button class="promo-btn"><img src="' + pieceImg + '"></button>');
    btn.on("click", function () {
      $("#promo-overlay").remove();
      var move = game.move({ from: source, to: target, promotion: p });
      if (move) {
        board.position(game.fen(), true);
        highlightLastMove(move);
        if (game.game_over()) {
          if (game.in_checkmate()) {
            var winner = game.turn() === "w" ? "Black" : "White";
            setStatus("Checkmate — " + winner + " wins!");
          } else {
            setStatus("Game over — draw");
          }
        } else {
          requestEngineMove();
        }
      }
    });
    dialog.append(btn);
  });

  overlay.append(dialog);
  $("body").append(overlay);
}

function onSnapEnd() {
  board.position(game.fen());
}

var config = {
  draggable: true,
  position: "start",
  pieceTheme: "https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png",
  onDragStart: onDragStart,
  onDrop: onDrop,
  onSnapEnd: onSnapEnd,
};

board = Chessboard("board", config);
$(window).resize(board.resize);

$("#newGameBtn").on("click", function () {
  game = new Chess();
  board.start(true);
  $(".highlight-square").removeClass("highlight-square");
  updateInfo(null, null, null);
  if (engineReady) {
    send("ucinewgame");
    sendPosition();
    if (playerColor === "b") {
      requestEngineMove();
    } else {
      setStatus("Your turn — White");
    }
  }
});

$("#flipBtn").on("click", function () {
  board.flip();
  playerColor = playerColor === "w" ? "b" : "w";
  game = new Chess();
  board.start(true);
  $(".highlight-square").removeClass("highlight-square");
  updateInfo(null, null, null);
  if (engineReady) {
    send("ucinewgame");
    sendPosition();
    if (playerColor === "b") {
      requestEngineMove();
    } else {
      setStatus("Your turn — " + (playerColor === "w" ? "White" : "Black"));
    }
  }
});

connectEngine();
