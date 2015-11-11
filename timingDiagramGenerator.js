(function gen(N, M) {
  var output = {
    signal: [],
    edge: []
  }

  for (var p = 0; p < N; p++) {
    var signal = {
      name: "node " + (p + 1),
      wave: 'z',
      data: [],
    }

    for (var step = 0 ; step < 2 * M - 1 ; step++) {
      if (step % 2 == 0) {
        signal.wave += "4..";
        signal.data.push('τ' + (1 + p + step/2*(N)));
      } else {
        signal.wave += "5.";
        signal.data.push('Com');
      }
    }

    for (var peer = 1; peer < N; peer++) {
      if(p == 0) {
        signal.wave += '3.';
        signal.data.push('Read n' + (peer + 1));
      } else if(peer == p) {
        signal.wave += '3.';
        signal.data.push('Send');
      } else {
        signal.wave += 'z.';
      }
    } 
    output.signal.push(signal);
  }

  return output;
})(10,5)
