var mochaChessApp = angular.module('mochaChessApp', ['ngRoute']);

mochaChessApp.config(function ($routeProvider) {
	$routeProvider

	.when('/', {
		templateUrl: 'pages/home.html',
		controller: 'mainController'
	})

	.when('/playComputerOptions', {
		templateUrl: 'pages/playComputerOptions.html',
		controller: 'playComputerOptionsController'
	})

	.when('/playComputer', {
		templateUrl: 'pages/playComputer.html',
		controller: 'playComputerController'
	})

	.when('/solveProblem', {
		templateUrl: 'pages/solveProblem.html',
		controller: 'solveProblemController'
	});
});

mochaChessApp.factory('PlayerData', function () {
	return { color: 'white' };
})

mochaChessApp.controller('mainController', function ($scope) {

});

mochaChessApp.controller('playComputerOptionsController', function ($scope, PlayerData) {
	$scope.player = PlayerData;
});


mochaChessApp.controller('playComputerController', function ($scope, PlayerData) {
	$scope.player = PlayerData;
	$scope.$on('$includeContentLoaded', function () {
		MochaChessApp.Application.startGame($scope.player.color == "white");
	});
});

function solveProblem() {
	MochaChessApp.Application.solveProblem(0);
}

mochaChessApp.controller('solveProblemController', function ($scope, $http) {
	var contentLoaded = false, dataLoaded = false;
	$http.get('moves.json').then(function (response) {
		dataLoaded = true;
		MochaChessApp.Application.setPositions(response.data);
		if(contentLoaded && dataLoaded)
		{
			solveProblem();
		}
	});
	$scope.$on('$includeContentLoaded', function () {
		contentLoaded = true;
		if (contentLoaded && dataLoaded)
		{
			solveProblem();
		}
	});
});