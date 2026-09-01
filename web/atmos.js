const $ = (selector, root = document) => root.querySelector(selector);
const $$ = (selector, root = document) => [...root.querySelectorAll(selector)];
const state = { selected: null, analysis: null, territory: null, spatial: null, compareLocations: [], rsTerritories: [] };

function localDate(daysAgo = 0) {
  const date = new Date();
  date.setDate(date.getDate() - daysAgo);
  return `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-${String(date.getDate()).padStart(2, '0')}`;
}

function applyPeriod(prefix, days = 30) {
  const start = prefix ? $(`#${prefix}start`) : $('#start-date');
  const end = prefix ? $(`#${prefix}end`) : $('#end-date');
  start.value = localDate(days);
  end.value = localDate(0);
}

applyPeriod('', 30);
applyPeriod('compare-', 30);

async function loadRsTerritories() {
  try {
    const response = await fetch('/assets/rs-territories.csv');
    if (!response.ok) throw new Error('catálogo indisponível');
    const [header, ...lines] = (await response.text()).trim().split(/\r?\n/).map(line => line.split(','));
    state.rsTerritories = lines.map(values => Object.fromEntries(header.map((key, index) => [key, values[index]])));
    $('#territory-catalog-status').textContent = `${state.rsTerritories.length} municípios · 28 COREDEs · 9 Regiões Funcionais · Pampa e Mata Atlântica.`;
    populateTerritoryUnits();
  } catch (error) {
    $('#territory-catalog-status').textContent = `Falha ao carregar a classificação territorial: ${error.message}`;
  }
}

function populateTerritoryUnits() {
  const scope = $('#territory-scope').value;
  const units = scope === 'municipio'
    ? state.rsTerritories.map(item => item.municipio)
    : [...new Set(state.rsTerritories.map(item => item[scope]))].sort((a, b) => a.localeCompare(b, 'pt-BR'));
  $('#territory-unit').innerHTML = units.map(unit => `<option value="${escapeHtml(unit)}">${escapeHtml(unit)}</option>`).join('');
  populateTerritoryMunicipalities();
}

function populateTerritoryMunicipalities() {
  const scope = $('#territory-scope').value;
  const unit = $('#territory-unit').value;
  const municipalities = scope === 'municipio'
    ? state.rsTerritories.filter(item => item.municipio === unit)
    : state.rsTerritories.filter(item => item[scope] === unit);
  $('#territory-municipalities').innerHTML = municipalities.map((item, index) => `<option value="${item.codigo_ibge}" ${index < Math.min(4, municipalities.length) ? 'selected' : ''}>${escapeHtml(item.municipio)}</option>`).join('');
}

$('#territory-scope').addEventListener('change', populateTerritoryUnits);
$('#territory-unit').addEventListener('change', populateTerritoryMunicipalities);
$('#add-territory-selection').addEventListener('click', () => {
  const codes = [...$('#territory-municipalities').selectedOptions].map(option => option.value);
  state.compareLocations = codes.slice(0, 4).map(code => {
    const item = state.rsTerritories.find(row => row.codigo_ibge === code);
    return { id:item.codigo_ibge, name:item.municipio, admin1:'Rio Grande do Sul', country:'Brasil', latitude:Number(item.latitude), longitude:Number(item.longitude) };
  });
  renderCompareChips();
});

loadRsTerritories();

function setView(name) {
  $$('.view').forEach(view => view.classList.toggle('active', view.id === `view-${name}`));
  $$('[data-view-link]').forEach(link => link.classList.toggle('selected', link.dataset.viewLink === name));
  history.replaceState(null, '', `#${name}`);
  window.scrollTo({ top: 0, behavior: 'smooth' });
}

$$('[data-view-link]').forEach(link => link.addEventListener('click', event => {
  event.preventDefault();
  setView(link.dataset.viewLink);
}));

const initialView = location.hash.slice(1);
if (['explorador', 'operacao', 'metodo'].includes(initialView)) setView(initialView);

$('#period-preset').addEventListener('change', event => {
  if (event.target.value === 'month') {
    const date = new Date();
    $('#start-date').value = `${date.getFullYear()}-${String(date.getMonth() + 1).padStart(2, '0')}-01`;
    $('#end-date').value = localDate();
  } else applyPeriod('', Number(event.target.value));
});

$$('input[name="product"]').forEach(input => input.addEventListener('change', () => {
  const product = $('input[name="product"]:checked').value;
  $('#method-hint').innerHTML = product === 'era5'
    ? '<strong>ERA5 consolidado</strong><p>Produto estável para comparação. Datas recentes podem ainda não estar disponíveis.</p>'
    : product === 'nasa_power'
      ? '<strong>NASA POWER · PRECTOTCORR</strong><p>Série pontual associada a produtos como MERRA-2; não representa uma estação.</p>'
      : '<strong>Best Match dinâmico</strong><p>Prioriza atualidade. O produto efetivo pode variar conforme data e local.</p>';
}));

async function api(url, options) {
  const response = await fetch(url, options);
  let payload;
  try { payload = await response.json(); } catch { payload = {}; }
  if (!response.ok) throw new Error(payload.message || 'O serviço não conseguiu processar a solicitação.');
  return payload;
}

function locationLabel(location) {
  return [location.name, location.admin1, location.country].filter(Boolean).join(' · ');
}

async function searchLocations(input, output, onSelect) {
  const query = input.value.trim();
  if (query.length < 2) return;
  output.hidden = false;
  output.innerHTML = '<button type="button"><small>Buscando localidades…</small></button>';
  try {
    const { locations } = await api(`/api/locations?q=${encodeURIComponent(query)}`);
    if (!locations.length) {
      output.innerHTML = '<button type="button"><small>Nenhuma localidade encontrada.</small></button>';
      return;
    }
    output.innerHTML = locations.map((item, index) => `<button type="button" data-index="${index}"><strong>${escapeHtml(item.name)}</strong><small>${escapeHtml([item.admin1, item.country].filter(Boolean).join(' · '))}</small></button>`).join('');
    $$('button[data-index]', output).forEach(button => button.addEventListener('click', () => {
      onSelect(locations[Number(button.dataset.index)]);
      output.hidden = true;
    }));
  } catch (error) {
    output.innerHTML = `<button type="button"><small>${escapeHtml(error.message)}</small></button>`;
  }
}

function escapeHtml(value) {
  return String(value).replace(/[&<>'"]/g, char => ({ '&':'&amp;', '<':'&lt;', '>':'&gt;', "'":'&#39;', '"':'&quot;' }[char]));
}

$('#search-location').addEventListener('click', () => searchLocations($('#location-search'), $('#location-results'), selectExplorerLocation));
$('#location-search').addEventListener('keydown', event => {
  if (event.key === 'Enter') { event.preventDefault(); searchLocations(event.target, $('#location-results'), selectExplorerLocation); }
});

function selectExplorerLocation(location) {
  state.selected = location;
  $('#location-search').value = location.name;
  $('#latitude').value = location.latitude;
  $('#longitude').value = location.longitude;
  $('#selected-location').textContent = `⌖ ${locationLabel(location)}`;
  $('#selected-location').hidden = false;
}

function showResultState(name) {
  $('#welcome-state').hidden = name !== 'welcome';
  $('#loading-state').hidden = name !== 'loading';
  $('#error-state').hidden = name !== 'error';
  $('#analysis-results').hidden = name !== 'result';
}

$('#analysis-form').addEventListener('submit', async event => {
  event.preventDefault();
  if (!state.selected) {
    $('#error-message').textContent = 'Selecione uma localidade na lista de resultados da busca.';
    showResultState('error');
    return;
  }
  showResultState('loading');
  try {
    const payload = {
      latitude: state.selected.latitude,
      longitude: state.selected.longitude,
      start_date: $('#start-date').value,
      end_date: $('#end-date').value,
      requested_product: $('input[name="product"]:checked').value
    };
    const analysis = await api('/api/analyses/precipitation', { method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(payload) });
    state.analysis = analysis;
    renderAnalysis(analysis);
    showResultState('result');
  } catch (error) {
    $('#error-message').textContent = error.message;
    showResultState('error');
  }
});

const number = value => new Intl.NumberFormat('pt-BR', { minimumFractionDigits:1, maximumFractionDigits:1 }).format(value);
const dateLabel = value => new Intl.DateTimeFormat('pt-BR', { timeZone:'UTC', day:'2-digit', month:'short' }).format(new Date(`${value}T00:00:00Z`));

function renderAnalysis(data) {
  $('#result-place').textContent = locationLabel(state.selected);
  $('#result-attribution').textContent = `${data.provenance.provider} · ${data.provenance.license} · estimativa modelada`;
  $('#product-badge').textContent = data.used_product === 'era5' ? 'ERA5 CONSOLIDADO' : data.used_product === 'nasa_power_merra2' ? 'NASA POWER' : 'BEST MATCH';
  $('#kpi-total').textContent = `${number(data.summary.total_mm)} mm`;
  $('#kpi-max').textContent = `${number(data.summary.maximum_daily_mm)} mm`;
  $('#kpi-mean').textContent = `${number(data.summary.mean_daily_mm)} mm`;
  $('#kpi-rainy').textContent = String(data.summary.rainy_days);
  const coverage = $('#coverage-strip');
  coverage.classList.toggle('partial', !data.complete);
  $('.coverage-icon', coverage).textContent = data.complete ? '✓' : '!';
  $('#coverage-title').textContent = data.complete ? 'Cobertura temporal completa' : 'Cobertura temporal parcial';
  $('#coverage-detail').textContent = `${data.available_days} dias disponíveis · período efetivo de ${dateLabel(data.effective_period.start)} a ${dateLabel(data.effective_period.end)}${data.missing_dates.length ? ` · ${data.missing_dates.length} ausências preservadas` : ''}`;
  renderBars($('#daily-chart'), data.daily, 'precipitation_mm');
  renderLine($('#accumulated-chart'), data.daily, 'accumulated_mm');
  $('#daily-table').innerHTML = data.daily.map(row => `<tr><td>${escapeHtml(dateLabel(row.date))}</td><td>${number(row.precipitation_mm)}</td><td>${number(row.accumulated_mm)}</td><td><span class="rain-tag">${rainClass(row.precipitation_mm)}</span></td></tr>`).join('');
  const provenance = [
    ['Produto solicitado', data.requested_product], ['Produto usado', data.used_product], ['Fonte', data.provenance.provider],
    ['Variável', data.provenance.variable], ['Licença', data.provenance.license], ['Método', data.provenance.method]
  ];
  $('#provenance-list').innerHTML = provenance.map(([term, value]) => `<div><dt>${escapeHtml(term)}</dt><dd>${escapeHtml(value)}</dd></div>`).join('');
  renderTerritoryMap();
}

function geometryRings(geometry) {
  if (!geometry) return [];
  if (geometry.type === 'Polygon') return geometry.coordinates;
  if (geometry.type === 'MultiPolygon') return geometry.coordinates.flat();
  return [];
}

async function renderTerritoryMap() {
  const target = $('#territory-map');
  $('#download-geojson').hidden = true;
  $('#download-spatial').hidden = true;
  state.territory = null;
  state.spatial = null;
  if (!state.selected || state.selected.country !== 'Brasil') {
    target.innerHTML = '<div class="map-loading">Malha municipal disponível para localidades brasileiras.</div>';
    return;
  }
  target.innerHTML = '<div class="map-loading">Consultando a malha oficial do IBGE…</div>';
  try {
    const territoryPromise = api(`/api/territories/municipality?name=${encodeURIComponent(state.selected.name)}&admin1=${encodeURIComponent(state.selected.admin1)}`);
    const spatialPromise = state.analysis.requested_product === 'best_match'
      ? api('/api/analyses/precipitation/spatial', { method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({ latitude:state.selected.latitude, longitude:state.selected.longitude, start_date:state.analysis.requested_period.start, end_date:state.analysis.requested_period.end, requested_product:'best_match', municipality_name:state.selected.name, admin1:state.selected.admin1 }) })
      : Promise.resolve(null);
    const [territory, spatial] = await Promise.all([territoryPromise, spatialPromise]);
    state.territory = territory;
    const features = territory.boundary.type === 'FeatureCollection' ? territory.boundary.features : [{ geometry: territory.boundary }];
    const rings = features.flatMap(feature => geometryRings(feature.geometry));
    const points = rings.flat();
    if (!points.length) throw new Error('A malha retornada não possui coordenadas.');
    const xs = points.map(point => point[0]), ys = points.map(point => point[1]);
    const minX = Math.min(...xs), maxX = Math.max(...xs), minY = Math.min(...ys), maxY = Math.max(...ys);
    const width = 900, height = 330, padding = 22;
    const scale = Math.min((width - 2 * padding) / Math.max(maxX - minX, .001), (height - 2 * padding) / Math.max(maxY - minY, .001));
    const project = ([lon, lat]) => [padding + (lon - minX) * scale + ((width - 2 * padding) - (maxX - minX) * scale) / 2, height - padding - (lat - minY) * scale - ((height - 2 * padding) - (maxY - minY) * scale) / 2];
    const paths = rings.map(ring => `M ${ring.map(point => project(point).join(' ')).join(' L ')} Z`).join(' ');
    const [pointX, pointY] = project([state.selected.longitude, state.selected.latitude]);
    const filteredCells = spatial ? spatial.cells.filter(cell => features.some(feature => pointInGeometry([cell.longitude, cell.latitude], feature.geometry))) : [];
    if (spatial) state.spatial = { ...spatial, cells:filteredCells };
    if (spatial) $('#map-note').textContent = spatial.adaptive_sampling
      ? 'Best Match: recorte extenso com adaptação explícita H3 r5→r5. Síntese amostral; não é superfície meteorológica nativa.'
      : 'Best Match: média de pontos únicos H3 r6 em células visuais r5. Síntese amostral; não é superfície meteorológica nativa.';
    const maxRain = Math.max(...filteredCells.map(cell => cell.precipitation_mm), 1);
    const cells = filteredCells.map(cell => {
      const d = `M ${cell.boundary.map(point => project(point).join(' ')).join(' L ')} Z`;
      const alpha = .18 + .67 * cell.precipitation_mm / maxRain;
      return `<path class="spatial-cell" d="${d}" fill="rgba(26,125,196,${alpha})"><title>${number(cell.precipitation_mm)} mm · ${escapeHtml(cell.h3_index)}</title></path>`;
    }).join('');
    target.innerHTML = `<svg viewBox="0 0 ${width} ${height}" role="img" aria-label="Limite e distribuição espacial de ${escapeHtml(territory.name)}"><defs><clipPath id="territory-clip"><path d="${paths}"/></clipPath></defs><path class="territory-shape" d="${paths}"/><g clip-path="url(#territory-clip)">${cells}</g><path class="territory-shape" d="${paths}" fill="none"/><circle class="territory-point" cx="${pointX}" cy="${pointY}" r="7"><title>Ponto da série meteorológica</title></circle></svg>`;
    $('#download-geojson').hidden = false;
    $('#download-spatial').hidden = !filteredCells.length;
  } catch (error) {
    target.innerHTML = `<div class="map-loading">${escapeHtml(error.message)}</div>`;
  }
}

function pointInRing([x, y], ring) {
  let inside = false;
  for (let i = 0, j = ring.length - 1; i < ring.length; j = i++) {
    const [xi, yi] = ring[i], [xj, yj] = ring[j];
    if (((yi > y) !== (yj > y)) && x < (xj - xi) * (y - yi) / ((yj - yi) || Number.EPSILON) + xi) inside = !inside;
  }
  return inside;
}

function pointInGeometry(point, geometry) {
  const polygonContains = polygon => pointInRing(point, polygon[0]) && !polygon.slice(1).some(hole => pointInRing(point, hole));
  if (geometry.type === 'Polygon') return polygonContains(geometry.coordinates);
  if (geometry.type === 'MultiPolygon') return geometry.coordinates.some(polygonContains);
  return false;
}

$('#download-geojson').addEventListener('click', () => {
  if (!state.territory) return;
  const blob = new Blob([JSON.stringify(state.territory.boundary)], { type:'application/geo+json' });
  const link = document.createElement('a');
  link.href = URL.createObjectURL(blob);
  link.download = `limite_ibge_${state.territory.code}.geojson`;
  link.click(); URL.revokeObjectURL(link.href);
});

$('#download-spatial').addEventListener('click', () => {
  if (!state.spatial) return;
  const geojson = { type:'FeatureCollection', features:state.spatial.cells.map(cell => ({
    type:'Feature', properties:{ h3_index:cell.h3_index, precipitation_mm:cell.precipitation_mm, analysis_mode:state.spatial.analysis_mode, surface_kind:'operational_sampling', spatial_aggregation:state.spatial.spatial_aggregation, sampling_resolution:state.spatial.sampling_resolution, presentation_resolution:state.spatial.presentation_resolution, adaptive_sampling:state.spatial.adaptive_sampling },
    geometry:{ type:'Polygon', coordinates:[[...cell.boundary, cell.boundary[0]]] }
  })) };
  const blob = new Blob([JSON.stringify(geojson)], { type:'application/geo+json' });
  const link = document.createElement('a'); link.href = URL.createObjectURL(blob);
  link.download = `amostragem_operacional_h3_${state.territory.code}.geojson`;
  link.click(); URL.revokeObjectURL(link.href);
});

function rainClass(mm) {
  if (mm < 1) return 'Sem chuva';
  if (mm < 10) return 'Leve';
  if (mm < 25) return 'Moderada';
  return 'Forte';
}

function chartFrame(values) {
  const width = 640, height = 220, left = 38, right = 10, top = 12, bottom = 28;
  const max = Math.max(...values, 1);
  return { width, height, left, right, top, bottom, plotW:width-left-right, plotH:height-top-bottom, max };
}

function axes(frame, labels) {
  let result = '';
  for (let i = 0; i <= 4; i++) {
    const y = frame.top + frame.plotH * i / 4;
    result += `<line class="grid-line" x1="${frame.left}" y1="${y}" x2="${frame.width-frame.right}" y2="${y}"/><text x="2" y="${y+3}">${number(frame.max*(1-i/4))}</text>`;
  }
  const labelIndexes = [...new Set([0, Math.floor((labels.length-1)/2), labels.length-1])];
  labelIndexes.forEach(index => {
    const x = frame.left + (labels.length <= 1 ? frame.plotW/2 : frame.plotW*index/(labels.length-1));
    result += `<text x="${x}" y="${frame.height-5}" text-anchor="middle">${escapeHtml(dateLabel(labels[index]))}</text>`;
  });
  return result;
}

function renderBars(target, rows, key) {
  const values = rows.map(row => row[key]);
  const f = chartFrame(values);
  const slot = f.plotW / Math.max(rows.length, 1);
  const bars = rows.map((row, index) => {
    const h = row[key] / f.max * f.plotH;
    return `<rect class="bar" x="${f.left+index*slot+slot*.12}" y="${f.top+f.plotH-h}" width="${Math.max(slot*.76,1)}" height="${h}" rx="2"><title>${dateLabel(row.date)}: ${number(row[key])} mm</title></rect>`;
  }).join('');
  target.innerHTML = `<svg viewBox="0 0 ${f.width} ${f.height}" role="img" aria-label="Gráfico de precipitação diária"><defs><linearGradient id="barGradient" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#2ea8e8"/><stop offset="1" stop-color="#1a7dc4"/></linearGradient></defs>${axes(f, rows.map(r=>r.date))}${bars}</svg>`;
}

function renderLine(target, rows, key) {
  const values = rows.map(row => row[key]);
  const f = chartFrame(values);
  const points = rows.map((row, index) => `${f.left+(rows.length<=1?f.plotW/2:f.plotW*index/(rows.length-1))},${f.top+f.plotH-row[key]/f.max*f.plotH}`).join(' ');
  const area = `${f.left},${f.top+f.plotH} ${points} ${f.width-f.right},${f.top+f.plotH}`;
  target.innerHTML = `<svg viewBox="0 0 ${f.width} ${f.height}" role="img" aria-label="Gráfico de precipitação acumulada">${axes(f, rows.map(r=>r.date))}<polygon class="area" points="${area}"/><polyline class="line" points="${points}"/></svg>`;
}

$('#toggle-table').addEventListener('click', () => {
  const table = $('#table-wrap');
  table.hidden = !table.hidden;
  $('#toggle-table').textContent = table.hidden ? 'Mostrar tabela' : 'Ocultar tabela';
});

$('#download-csv').addEventListener('click', () => {
  if (!state.analysis) return;
  const lines = ['data,precipitacao_mm,acumulado_mm', ...state.analysis.daily.map(row => `${row.date},${row.precipitation_mm},${row.accumulated_mm}`)];
  const blob = new Blob([lines.join('\n')], { type:'text/csv;charset=utf-8' });
  const link = document.createElement('a');
  link.href = URL.createObjectURL(blob);
  link.download = `atmos_precipitacao_${state.selected.name.replace(/\W+/g,'_').toLowerCase()}.csv`;
  link.click(); URL.revokeObjectURL(link.href);
});

function addCompareLocation(location) {
  if (state.compareLocations.some(item => item.id === location.id) || state.compareLocations.length >= 4) return;
  state.compareLocations.push(location);
  renderCompareChips();
  $('#compare-search').value = '';
}

function renderCompareChips() {
  $('#compare-locations').innerHTML = state.compareLocations.map((item, index) => `<span class="compare-chip">${escapeHtml(item.name)}<button type="button" data-remove="${index}" aria-label="Remover">×</button></span>`).join('');
  $$('[data-remove]', $('#compare-locations')).forEach(button => button.addEventListener('click', () => {
    state.compareLocations.splice(Number(button.dataset.remove), 1); renderCompareChips();
  }));
}

$('#compare-search-button').addEventListener('click', () => searchLocations($('#compare-search'), $('#compare-results'), addCompareLocation));
$('#compare-search').addEventListener('keydown', event => { if (event.key === 'Enter') { event.preventDefault(); searchLocations(event.target, $('#compare-results'), addCompareLocation); } });

$('#compare-form').addEventListener('submit', async event => {
  event.preventDefault();
  if (state.compareLocations.length < 2) return;
  $('#compare-empty').hidden = false;
  $('#compare-empty').innerHTML = '<div class="loader"></div><h2>Construindo comparação</h2>';
  $('#compare-output').hidden = true;
  try {
    const analyses = await Promise.all(state.compareLocations.map(location => api('/api/analyses/precipitation', {
      method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({ latitude:location.latitude, longitude:location.longitude, start_date:$('#compare-start').value, end_date:$('#compare-end').value, requested_product:'era5' })
    })));
    const max = Math.max(...analyses.map(item => item.summary.total_mm), 1);
    $('#compare-output').innerHTML = analyses.map((item, index) => `<article class="panel compare-card"><h2>${escapeHtml(locationLabel(state.compareLocations[index]))}</h2><p>${item.available_days} dias · ${escapeHtml(item.used_product)}</p><div class="compare-total">${number(item.summary.total_mm)} mm</div><div class="compare-bar"><span style="width:${item.summary.total_mm/max*100}%"></span></div></article>`).join('');
    $('#compare-empty').hidden = true;
    $('#compare-output').hidden = false;
  } catch (error) {
    $('#compare-empty').innerHTML = `<h2>Comparação indisponível</h2><p>${escapeHtml(error.message)}</p>`;
  }
});
